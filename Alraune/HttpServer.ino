/*
 * Start the webserver and register handlers for all ressources
 * 
 * All static ressources are handled through the handleFileRead handler
 * The endpoints for data or commands are handled by individual handlers per endpoint
 * 
 * After the server has started the mDNS service is informed about the new http service
 */
void setupServer(){
  Serial.println(F("Configuring web server"));

  server.on(F("/"), HTTP_GET, handleFileRead);
  server.on(F("/"), HTTP_OPTIONS, sendCrossOriginHeader);
  server.on(F("/index.html"), HTTP_GET, handleFileRead);
  server.on(F("/index.html"), HTTP_OPTIONS, sendCrossOriginHeader);
  server.on(F("/alraune_ico.jpg"), HTTP_GET, handleFileRead);
  server.on(F("/alraune.jpg"), HTTP_GET, handleFileRead);
  server.on(F("/alraune.png"), HTTP_GET, handleFileRead);
  server.on(F("/topf_oben.png"), HTTP_GET, handleFileRead);
  server.on(F("/topf_unten.png"), HTTP_GET, handleFileRead);
  server.on(F("/alraune.css"), HTTP_GET, handleFileRead);
  server.on(F("/alraune.js"), HTTP_GET, handleFileRead);
  server.on(F("/setup.js"), HTTP_GET, handleFileRead);
  server.on(F("/tickle.mp3"), HTTP_GET, handleFileRead);
  
  server.on(F("/json"), HTTP_GET, handleJson);
  server.on(F("/wlan/scan"), HTTP_GET, handleNetworkScan);
  server.on(F("/wlan/setup"), HTTP_POST, handleNetworkSetup);
  server.on(F("/wlan/status"), HTTP_GET, handleNetworkStatus);
  server.on(F("/wlan/status"), HTTP_OPTIONS, sendCrossOriginHeader);
  server.on(F("/play"), HTTP_POST, handlePlaySound);
  server.on(F("/recalibrate"), HTTP_POST, handleSensorRecalibration);
  server.on(F("/volume"), HTTP_POST, handleVolume);
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  ip=WiFi.localIP().toString();
  MDNS.addService("http", "tcp", 80);
  log("Web server started");
}

/*
 * Check if there are any requests and handle them
 */
void handleHttpRequests(){
  server.handleClient();
}

/*
 * Handler for the JSON endpoint
 * 
 * Answer with a json, that contains the sensor value and some status variables
 */
void handleJson() {
  Serial.println(F("Requested /json"));
  digitalWrite(led, 0);
  // The sensor ranges from 0-1000. The value is higher, if the soil is more dry. Invert the value, since we want to measure moisture not dryness
  sensorValue = 1000 - analogRead(sensorPin);

  JSONVar jsonObject;
  jsonObject["title"] = pagename;
  jsonObject["moisture"] = sensorValue;
  jsonObject["sensorThreshold"] = sensorThreshold;
  jsonObject["memory"] = (long) ESP.getFreeHeap();
  jsonObject["needsWater"] = needsWater;
  jsonObject["needsRefill"] = needsRefill;
  jsonObject["volume"] = volume;

  server.send(200, APPLICATION_JSON, JSON.stringify(jsonObject));
  digitalWrite(led, 255);
  Serial.println(F("JSON for /json sent"));
}

/*
 * Handler for network scans
 * 
 * This handler causes the system in Access Point mode to scan for wireless networks in the area
 * The result is stored as JSON Array in the global variable wlanListJson and send to the client
 */
void handleNetworkScan() {
  Serial.println(F("Requested /wlan/scan"));
  digitalWrite(led, 0);
  scan();
  Serial.println(wlanListJson);
  
  server.send(200, APPLICATION_JSON, wlanListJson);
  wlanListJson = "";
  digitalWrite(led, 255);
  Serial.println(F("JSON for /wlan/scan sent"));
}

/*
 * Handler for network status
 * 
 * This handler returns a JSON object, that tells the client, if the system is in AP mode or in another WLAN
 */
void handleNetworkStatus(){
  Serial.println(F("Requested /wlan/status"));
  digitalWrite(led, 0);

  JSONVar jsonObject;
  jsonObject["connectedToSsid"] = connectedToSsid;
  server.sendHeader(F("Access-Control-Allow-Origin"),F("http://alraune.local"));
  server.send(200, APPLICATION_JSON, JSON.stringify(jsonObject));
  digitalWrite(led, 255);
  Serial.println(F("JSON for /wlan/status sent"));
}

/*
 * Handler for system setup
 * 
 * This handler stores the provides information in the configuration file and sends a 202 - ACCEPTED
 * Then it restarts the whole system, so that it is booted with the new configuration
 * 
 * The JSON conversion checks for the required fields and will answer with a 400 - BAD REQUEST, if a field is missing
 */
void handleNetworkSetup() {
  Serial.println(F("Requested /wlan/setup"));
  digitalWrite(led, 0);

  if (server.hasArg("plain")== false){
    Serial.println(F("No body sent"));
 
    server.send(400, TEXT_PLAIN, "Invalid body");
    return;
  }
  JSONVar body = JSON.parse(server.arg("plain"));
  
  if (JSON.typeof(body) == "undefined") {
    Serial.println(F("Body is no valid JSON"));
 
    server.send(400, TEXT_PLAIN, "Invalid body");
    return;
  }
    
  if (!body.hasOwnProperty("name")) {
    Serial.println(F("JSON does not contain a name"));
 
    server.send(400, TEXT_PLAIN, "(String) name is reqired in the body");
    return;
  }
  
  if (!body.hasOwnProperty("ssid")) {
    Serial.println(F("JSON does not contain a ssid"));
 
    server.send(400, TEXT_PLAIN, "(String) ssid is reqired in the body");
    return;
  }
    
  if (!body.hasOwnProperty("password")) {
    Serial.println(F("JSON does not contain a password"));
 
    server.send(400, TEXT_PLAIN, "(String) password is reqired in the body");
    return;
  }
      
  if (!body.hasOwnProperty("birthdayDay")) {
    Serial.println(F("JSON does not contain a birthdayDay"));
 
    server.send(400, TEXT_PLAIN, "(int) birthdayDay is reqired in the body");
    return;
  }
      
  if (!body.hasOwnProperty("birthdayMonth")) {
    Serial.println(F("JSON does not contain a birthdayMonth"));
 
    server.send(400, TEXT_PLAIN, "(int) birthdayMonth is reqired in the body");
    return;
  }
  
  pagename = (const char*)body["name"];
  ssid = (const char*)body["ssid"];
  wlanPassword = (const char*)body["password"];
  birthdayDay = (int)body["birthdayDay"];
  birthdayMonth = (int)body["birthdayMonth"];
  writeConfigToFs();

  
  server.send(200, TEXT_PLAIN, "Config written");

  delay(20*1000);
  Serial.println(F("Reboot system via watchdog"));
  while(true);
}

  
/*
 * Handler for moisture sensor recallibration
 * 
 * Set the moisture treshold to the current level
 * You can readjust a sensor over the time with this method
 */
void handleSensorRecalibration(){
  Serial.println(F("Requested /recalibrate"));
  digitalWrite(led, 0);
  
  // The sensor ranges from 0-1000. The value is higher, if the soil is more dry. Invert the value, since we want to measure moisture not dryness
  sensorThreshold = 1000 - analogRead(sensorPin);
  String message = "Set sensor treshold to ";
  message += sensorThreshold;
  Serial.println(message);
  
  writeConfigToFs();
  
  server.sendHeader("Location","/");
  
  server.send(303);
  digitalWrite(led, 255);
  Serial.println(F("Redirected back to /"));
}

/*
 * Handler for volume level
 * 
 * Adjust volume level to the given value, if it is in the allowed range from 0-30
 * Otherwise nothing is done
 * 
 * The JSON conversion checks for the required fields and will answer with a 400 - BAD REQUEST, if a field is missing
 */
void handleVolume(){
  Serial.println(F("Requested /volume"));
  digitalWrite(led, 0);

  if (server.hasArg("plain")== false){
    Serial.println(F("No body sent"));
 
    server.send(400, TEXT_PLAIN, "Invalid body");
    return;
  }
  JSONVar body = JSON.parse(server.arg("plain"));
  
  if (JSON.typeof(body) == "undefined") {
    Serial.println(F("Body is no valid JSON"));
 
    server.send(400, TEXT_PLAIN, "Invalid body");
    return;
  }
  
  if (!body.hasOwnProperty("volume")) {
    Serial.println(F("JSON does not contain a volume"));
 
    server.send(400, TEXT_PLAIN, "(int) volume is reqired in the body");
    return;
  }
  
  int sendVolume = (int)body["volume"];
  if (sendVolume >= 0 && sendVolume <=30){
    Serial.print(F("Set volume to ")); 
    Serial.println(sendVolume);
    volume = sendVolume;
    myDFPlayer.volume(volume);
    
    writeConfigToFs();
  } else {
    Serial.println(F("Volume out of range. Doing nothing"));
  }
  
  
  server.send(200, TEXT_PLAIN, "Volume set");
  digitalWrite(led, 255);
  Serial.println(F("Volume set"));
}

/*
 * Handler for the demo
 * 
 * Plays one random sound of the requested category. The Alraune has water refill sounds, birthday sounds and christmas sounds
 * 
 * The JSON conversion checks for the required fields and will answer with a 400 - BAD REQUEST, if a field is missing
 */
void handlePlaySound(){
  Serial.println(F("Requested /play"));
  digitalWrite(led, 0);

  if (server.hasArg("plain")== false){
    Serial.println(F("No body sent"));
 
    server.send(400, TEXT_PLAIN, "Invalid body");
    return;
  }
  JSONVar body = JSON.parse(server.arg("plain"));
  
  if (JSON.typeof(body) == "undefined") {
    Serial.println(F("Body is no valid JSON"));
 
    server.send(400, TEXT_PLAIN, "Invalid body");
    return;
  }
  
  if (!body.hasOwnProperty("sound")) {
    Serial.println(F("JSON does not contain a sound"));
 
    server.send(400, TEXT_PLAIN, "(String) sound is reqired in the body");
    return;
  }
  
  String sound = (const char*)body["sound"];
  
  if (sound == "water"){
    playWaterRefillSound();
  } else if ( sound == "birthday") {
    playBirthdaySound();
  } else if ( sound == "christmas") {
    playChristmasSound();
  } else {
    Serial.println(F("Requested sound is unknown"));
 
    server.send(400, TEXT_PLAIN, "sound is unknown");
  }

  
  server.send(200, TEXT_PLAIN, "Playing");
  digitalWrite(led, 255);
  Serial.println(F("Played sound"));
}

/*
 * Handler for static files
 * 
 * The handler fetches the path to the requested file, loads it from the LittleFS and delivers the content of the file with the correct MIME type
 * 
 * If / is requested, the index.html is served instead. If the server is in Access Point mode, the setup.html is served instead of the index.html
 * 
 * If a file is not available, the gzipped version is used, if it exists
 * 
 * The max-age header is set to one day to increase performance but allow upgrades of the system
 * 
 * A 500 - INTERNAL SERVER ERROR is send, if the filesystem is not ready
 * A 404 - NOT FOUND is send, if the requested file does not exist
 */
void handleFileRead() {
  String path = ESP8266WebServer::urlDecode(server.uri());
  
  Serial.print(F("handleFileRead: "));
  Serial.println(path);
  if (!fsOK) {
    server.send(500, TEXT_PLAIN, "Filesystem not ready");
    return;
  }

  if (path.endsWith("/")) {
    path += "index.html";
  }

  if (path.equals("/index.html") && !connectedToSsid) {
    path = "/setup.html";
  }

  String contentType;
  if (server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
 
    server.sendHeader(F("Access-Control-Allow-Origin"),F("http://alraune.local"));
    server.sendHeader(F("max-age"),F("86400"));
    if (server.streamFile(file, contentType) != file.size()) {
      Serial.println(F("Sent less data than expected!"));
    }
    file.close();
  } else {
    handleNotFound();
  }
}

/*
 * Handler for 404 - FILE NOT FOUND errors
 * 
 * Answers paths, that have not handler registered, with a 404 - FILE NOT FOUND
 */
void handleNotFound() {
  digitalWrite(led, 0);
  String message = "File not found: ";
  String uri = ESP8266WebServer::urlDecode(server.uri()); 
  message += uri;
  log(message);
  
  server.send(404, TEXT_PLAIN, "File not found!\r\n");
  digitalWrite(led, 255);
}

/**
 * Send a CORS Header
 * 
 * Allows access from the mDNS address alraune.local to allow the setup page to check the homepage to check the success of the setup
 */
void sendCrossOriginHeader(){
    server.sendHeader(F("Access-Control-Allow-Origin"),F("http://alraune.local"));
    server.send(204);
}
