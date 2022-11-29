/*
 * Initialice the pins of the D1 Mini
 * 
 * The relayPin is used as output pin to control the pump
 * It has to be set to HIGH, to shut down the pump
 * The led pin is used to signal access to the webserver
 * Power it on for the setup, to signal that we are doing something
 */
void setupPins(){
  // Initialize the random generator with the input from the analog pin
  randomSeed(analogRead(A0));
  
  pinMode(releayPin, OUTPUT);
  digitalWrite(releayPin, HIGH);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 255);
}

/*
 * Start the serial interface 
 * 
 * Start the seerial interface with low baud rate for easy debugging, if the network logs are not available
 */
void setupSerial(){
  Serial.begin(9600);
  Serial.println(F("Serial inteface for debugging is initialized with 9600 baud."));
}

/*
 * Configure the upgrade interval
 * 
 * The upgrade interval is the time betweet two checks for an upgrade on the configured upgrade server
 */
void setupUpgrade(){
  upgradeDelay.start(3 * 60 * 60 * 1000);
}

/*
 * Upgrade the system and its files
 * 
 * After the upgrade interval passed a check against the upgrade server is done with the current version number
 * An updgrade is done, if the server has a newer version, than the system. The current version is stored in the global variable version
 * 
 * First the files are upgraded and then the firmware
 * 
 * The calls to the upgrade server are protected with TLS, therefore the HTTP client has already a trust ancor with the let's encrypt CA file
 * The calls also use basic authentication with the configures credentials for the upgrade server
 * The upgrade URL is the hostname of the upgrade server. This is configured in the config.json. This system requires it to have a TLS secured http server on port 443
 * The upgrade path is the path to the project folder on the upgade server. On my upgrade server this is /alraune/files/
 * A complete request for the new firmare could be https://my-upgrade-server.org/alraune/files/firmware.bin
 * 
 * The firmware file is automatically signed by the Arduino IDE and the signature is checked by the ESPhttpUpdate
 * An upgrade is denied if the signature does not match
 * The files on the LittleFS filesystem are not verified with a signature at the moment
 */
void upgrade(){
  if (upgradeDelay.justFinished()){
    upgradeDelay.repeat();

    upgradeFiles();
  
    #if MANUAL_SIGNING
      loadPublicSigningKey();
    #endif
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    ESPhttpUpdate.setAuthorization(upgradeUser, upgradePassword);
    
    Serial.println(F("Start checking for new updates"));
    String firmware = upgradePath + "firmware.bin";
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, upgradeUrl, 443, firmware, VERSION);
    handleUpdateResult(ret);
  }
}

/*
 * Upgrade the files
 * 
 * Checks for each file (see exceptions) on the LittleFS, if an upgrade is available and donwloads the new file version
 * 
 * Files, that are no longer on the server for a newer version will be deleted
 * 
 * Nothing is done for a file, if it is already the latest version
 * 
 * The calls to the upgrade server are protected with TLS, therefore the HTTP client has already a trust ancor with the let's encrypt CA file
 * The calls also use basic authentication with the configures credentials for the upgrade server
 * The upgrade URL is the hostname of the upgrade server. This is configured in the config.json. This system requires it to have a TLS secured http server on port 443
 * The upgrade path is the path to the project folder on the upgade server. On my upgrade server this is /alraune/files/
 * A complete request for a file could be https://my-upgrade-server.org/alraune/files/index.html.gz
 * 
 * The files on the LittleFS filesystem are not verified with a signature at the moment
 * 
 * Exceptions:
 * - config.json contains customized configurations. If an upgrade of a value is needed or a new value is added, this has to happen by adding it to the ConfigWriter
 * - tickle.mp3 the javascript accesses that file and can't handle gzip. But mp3 is already compressed 
 * - ca.cer since it is needed in the code and not served other HTTP
 */
void upgradeFiles(){
  HTTPClient http;
  http.setReuse(true);

  File root = LittleFS.open("/", "r");
  File file = root.openNextFile();
  while (file){
    String filename = file.name();
    String errorMessage;
    
    Serial.println(filename);
    if ( strcmp("config.json", file.name()) == 0 ){
      Serial.println(F("Skip update of config.json because it is customized per device"));
      file = root.openNextFile();
      continue;
    }
    if ( strcmp("tickle.mp3", file.name()) == 0 ){
      Serial.println(F("Skip update of tickle.mp3 because it is to big to upgrade"));
      file = root.openNextFile();
      continue;
    }
    String url = "https://";
    url += upgradeUrl;
    url += upgradePath;
    url += filename;
    http.begin(client, url);
    http.setUserAgent("curl/7.74.0");
    http.addHeader("Authorization", "Basic " + base64::encode(upgradeUser + ":" + upgradePassword));
    http.addHeader("x-ESP8266-version", VERSION);

    int httpCode = http.GET();

    switch(httpCode){
      case 200:
        Serial.print(F("Found new version of file "));
        Serial.print(filename);
        Serial.println(F(". Save the stream to the file"));
        file.close();
        file = LittleFS.open(filename, "w+");
        if (file){
          int fileCode = http.writeToStream(&file);
          if (fileCode <= 0){
            Serial.printf("File could not be written. The file is destroyed. Error code: %i\r\n", fileCode);
          } else {
            Serial.printf("File upgraded. Filesize is: %i\r\n", fileCode);
          }
          file.close();
        }
        break;
      case 304:
        Serial.println(F("File is already the latest version"));
        break;
      case 404:
        Serial.println(F("File not found in new verion. Delete it"));
        file.close();
        LittleFS.remove(filename);
        break;
      default:
        errorMessage = "Could not check for updates. HTTP Code was ";
        errorMessage += httpCode;   
        errorMessage += ". Network status is ";
        errorMessage += WiFi.status();
        Serial.println(errorMessage);
        break;
    }    
    file = root.openNextFile();
  }
  http.end();
  Serial.println(F("Finished upgrade of files."));
}

/*
 * Loads the public key for signature verification
 * 
 * The firmware file is automatically signed by the Arduino IDE and the signature is checked by the ESPhttpUpdate
 * This can be switched to manual singing and verification by setting MANUAL_SIGNING to 1
 * In manual signing mode the public key is read from the LittleFS and converted to a SigningVerifier
 */
#if MANUAL_SIGNING
  void loadPublicSigningKey(){
    File publikKeyFile = LittleFS.open("public.key", "r");
    if(!publikKeyFile) {
      Serial.println(F("Couldn't load public key. Can't upgrade"));
    } else {
      char *publikKey;
      size_t publikKeySize = publikKeyFile.size();
      publikKey = (char *)malloc(publikKeySize);
      if (publikKeySize != publikKeyFile.readBytes(publikKey, publikKeySize)) {
        Serial.println(F("Loading public key failed. Can't upgrade"));
      } else {
        Serial.println(F("Loaded public key"));
        signPubKey = new BearSSL::PublicKey(publikKey);
        hash = new BearSSL::HashSHA256();
        sign = new BearSSL::SigningVerifier(signPubKey);
      }
      free(publikKey);
      publikKeyFile.close();
    }
    Update.installSignature(hash, sign);
  }
#endif

/*
 * Print the result of a firmware upgrade
 * 
 * Prints the result of a firmware upgrade, if it was not successfull
 * Otherwise the system reboots before we can enter this function
 */  
void handleUpdateResult(t_httpUpdate_return ret){
  String logMessage;
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      logMessage = "Firmware update canceled. Error (";
      logMessage += ESPhttpUpdate.getLastError();
      logMessage += "): ";
      logMessage += ESPhttpUpdate.getLastErrorString().c_str();
      break;
    case HTTP_UPDATE_NO_UPDATES:
      logMessage = "Already using the latest firmware. No update needed";
      break;
    case HTTP_UPDATE_OK:
      logMessage = "Firmware update done";
      break;
  }
  log(logMessage);
}
