/*
 * Start the Wireless LAN connection to the configured SSID
 * 
 * Try to connect to the configured network
 * If the connection is successfull, also start the Multicast DNS server to be accessable through the configured hostname
 * And initialize the client and the network monitoring
 * Sets the global variable connectedToSsid to true
 * 
 * If the connection could not be established the global variable connectedToSsid is set to false
 * In that case, an Access Point with the name alraune is started
 */
void setupNetwork(){
  Serial.println(F("Start WIFI connection"));
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, wlanPassword);
  
  if (testWifi())
  {
    Serial.println(F("Succesfully Connected"));
    initClient();  
    networkMonitoringDelay.start(1000*10);
    connectedToSsid = true;
  } else {
    Serial.println(F("Could not connect. Turning the HotSpot On"));
    setupAP();
    connectedToSsid = false;
  }    
  if (MDNS.begin(hostname)) {
    Serial.println(F("mDNS responder started"));
  }
}

/*
 * Configures the HTTP client
 * 
 * The HTTP client needs to know the CA of let's encrypt to establish TLS encrypted connections to my servers
 * This method reads the ca.cer file and creates a X509List, that is used as a trust ancor for the client
 */
void initClient(){
  File ca = LittleFS.open("ca.cer", "r");
  if(!ca) {
    Serial.println(F("Couldn't load CA cert"));
  } else {
    char *ca_cert;
    BearSSL::X509List *rootCert;
    size_t certSize = ca.size();
    ca_cert = (char *)malloc(certSize);
    if (certSize != ca.readBytes(ca_cert, certSize)) {
      Serial.println(F("Loading CA cert failed"));
    } else {
      Serial.println(F("Loaded CA cert"));
      rootCert = new BearSSL::X509List(ca_cert);
      client.setTrustAnchors(rootCert);
    }
    free(ca_cert);
    ca.close();
  }
}

/*
 * Triggers the update of the mDNS service
 */
void updateDns(){
  MDNS.update();
}

/*
 * Check for network errors and restart the network if needed
 * 
 * Checks every 10 minutes, if the network is available
 * If it is connected, everything is fine
 * Otherwise the status is printed and the network setup function is called again
 */
void handleNetworErrorsIfNeeded(){
  if(networkMonitoringDelay.justFinished()){
    networkMonitoringDelay.repeat();
    switch (WiFi.status()){
      case WL_CONNECTED:
        break;
      default:
        String errorMessage = "Network error detected: ";
        errorMessage += WiFi.status();
        errorMessage += ". Trying to reconnect. If it fails, the device will reboot in 3 Minutes";
        log(errorMessage);
        setupNetwork();
        Serial.println(F("Network reconnected"));
        break;
    } 
  }
}

/*
 * Tests if the connection to a WLAN was successfull
 * 
 * Returns true, if the connection was established and the status was WL_CONNECTED
 * Returns false otherwise
 */
bool testWifi(){
  int counter = 0;
  Serial.println(F("Waiting for Wifi to connect"));
  while ( counter < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print(F("."));
    counter++;
  }
  Serial.println(F("Connect timed out, opening AP"));
  return false;
}

/*
 * Starts an own Access Point
 * 
 * Starts a standalone Access Point with the name alraune and the password WaterIsLive
 * Only one client can connect to the AP to prevent any unwanted guests during setup
 */
void setupAP()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.softAP("alraune", "WaterIsLive", 1, false, 1);
}

/*
 * Scans the area for public wireless networks
 * 
 * Scans for WLANs without hidden SSID and creates a JSON Array with the information
 * This JSON is stored in the global variable wlanListJson
 */
void scan(){
  int n = WiFi.scanNetworks();
  Serial.println(F("WLAN scan done"));
  if (n == 0) {
    Serial.println(F("No networks found"));
  }
  wlanListJson = "[";
  for (int i = 0; i < n; ++i)
  {
    wlanListJson += "{ \"ssid\": \"";
    wlanListJson += WiFi.SSID(i);
    wlanListJson += "\", \"rssi\": \"";
    wlanListJson += WiFi.RSSI(i); 
    wlanListJson += "\", \"encryption\": ";
    if ((WiFi.encryptionType(i) == ENC_TYPE_NONE)){
      wlanListJson += false;
    } else {
      wlanListJson += true;
    }
    if (i < n-1){
      wlanListJson += "},\n";
    } else {
      wlanListJson += "}";
    }
  }
  wlanListJson += "]";
}
