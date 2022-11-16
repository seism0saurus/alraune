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
void log(const String message){
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

/*
 * Log a status json to the log server
 * 
 * Every time the logDelay is finished, a status message is constructed and send to the configured logserver
 * Than the delay is repeated. The duration is one minute
 */
void logStatus(){
  if (logDelay.justFinished()){
    logDelay.repeat();
    
    String status = "{";
  
    status += "\"service\":\"alraune\",";
  
    status += "\"chipId\":\"";
    status += ESP.getChipId();
    status += "\",";
  
    status += "\"ssid\":\"";
    status += ssid;
    status += "\",";
    
    status += "\"internal_ip\":\"";
    status += ip;
    status += "\",";
  
    status += "\"port\":";
    status += port;
    status += ",";
    
    status += "\"hostname\":\"";
    status += hostname;
    status += ".local\",";
  
    char memoryString[8];
    sprintf(memoryString, "%d", ESP.getFreeHeap());
    status += "\"memory\":";
    status += memoryString;
    status += ",";
  
    status += "\"moistureThreshold\":";
    status += sensorThreshold;
    status += ",";
    
    char moistureString[4];
    sprintf(moistureString, "%d", sensorValue);
    status += "\"moisture\":";
    status += moistureString;
    status += ",";
    
    if (needsWater){
        status += "\"needsWater\":true,";
    } else {    
        status += "\"needsWater\":false,";
    }
  
    if (needsRefill){
        status += "\"needsRefill\":true,";
    } else {    
        status += "\"needsRefill\":false,";
    }

    status += "\"message\":\"Log statistic\"";
    
    status += "}";
    sendToLogServer(status); 
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
void sendToLogServer(String message){
  // Sanitize data. Logserver expects single line logs
  message.replace("\r\n","");
  message.replace("\n","");

  HTTPClient http;
  http.setReuse(false);
  http.begin(client, logUrl);
  String auth = base64::encode(logUser + ":" + logPassword); 
  http.addHeader("Authorization", "Basic " + auth);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(message);
  String payload = http.getString();

  if (httpCode == -1){
    String errorMessage = "Could not post logs. HTTP Code was ";
    errorMessage += httpCode;
    errorMessage += " - ";
    errorMessage += payload;    
    errorMessage += ". Network status is ";
    errorMessage += WiFi.status();
    Serial.println(errorMessage);
  } else {
    String debug = "Logged following payload to ";
    debug += logUrl;
    debug += " and got HTTP code ";
    debug += httpCode;
    debug += " - ";
    debug += payload;
    debug += ":\n";
    debug += message;
    debug += "\n";
    Serial.println(debug);
  }
  http.end();
}
