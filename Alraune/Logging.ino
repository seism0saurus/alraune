/*
 * Start logging status information every minute
 */
void setupLogging(){
  logDelay.start(1000*60);
}

/*
 * Log a message to the serial port and a webservice
 * 
 * Logging to the serial port is mandatory
 * 
 * The webservice is only used, if the Alraune is connected to a WLAN
 * Otherwise the system has no internet connectivity and can't log to a log server
 */
void log(const String &message){
  Serial.println(message);
  
  JSONVar jsonObject;
  jsonObject["service"] = "alraune";
  jsonObject["chipId"] = (long) ESP.getChipId();
  jsonObject["message"] = message;

  if (connectedToSsid){
    sendToLogServer(jsonObject);
  }
}
/*
void log(const __FlashStringHelper *message){
  Serial.println(message);
  
  String json = "{";
  
  json += "\"service\":\"alraune\",";

  json += "\"chipId\":\"";
  json += ESP.getChipId();
  json += "\",";
  
  json += "\"message\":\"";
  json += message;
  json += "\"}";

  if (connectedToSsid){
    sendToLogServer(json);
  }
}
*/

/*
 * Log a status json to the log server
 * 
 * Every time the logDelay is finished, a status message is constructed and send to the configured logserver
 * Than the delay is repeated. The duration is one minute
 */
void logStatus(){
  if (logDelay.justFinished()){
    logDelay.repeat();

    JSONVar jsonObject;
    jsonObject["service"] = "alraune";
    jsonObject["chipId"] = (long) ESP.getChipId();
    jsonObject["ssid"] = ssid;
    jsonObject["internal_ip"] = ip;
    jsonObject["port"] = port;
    jsonObject["hostname"] = hostname + F(".local");
    jsonObject["memory"] = (long) ESP.getFreeHeap();
    jsonObject["moistureThreshold"] = (int) sensorThreshold;
    jsonObject["moisture"] = sensorValue;
    jsonObject["needsWater"] = needsWater;
    jsonObject["needsRefill"] = needsRefill;
    jsonObject["message"] = "Log statistic";
    
    sendToLogServer(jsonObject); 
  } else {
    delay(1);
  }
}

/*
 * Wrapper method to handle the http requests to send a message to the logserver
 * 
 * It takes a json formatted message as String and starts a HTTPClient
 * The configured authentication credentials are used to authenticate against the logserver
 * 
 * If the http code is -1, there is a problem with the connection to the network
 * In that case a message with the network status is printed
 * Otherwise a debug message is printed to the serial port
 */
void sendToLogServer(const JSONVar& jsonObject){

  HTTPClient http;
  http.setReuse(false);
  http.begin(client, logUrl);
  String auth = base64::encode(logUser + ":" + logPassword); 
  http.addHeader("Authorization", "Basic " + auth);
  http.addHeader("Content-Type", APPLICATION_JSON);
  
  int httpCode = http.POST(JSON.stringify(jsonObject));
  String payload = http.getString();

  if (httpCode == -1){
    Serial.print(F("Could not post logs. HTTP Code was "));
    Serial.print(httpCode);
    Serial.print(F(" - "));
    Serial.print(payload);    
    Serial.print(F("Network status is "));
    Serial.println(WiFi.status());
  }
  http.end();
}
