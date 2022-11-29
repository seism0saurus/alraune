  /*
 * Stores the current configuration as JSON in the config.json file on the LittleFS filesystem
 * 
 * This overrides the current file
 * If the file could not be found or written, an error is printed in the serial interface
 * 
 * Example config.json:
 {
    "name": "will-be-set-during-setup",
    "wlan": {
        "ssid": "will-be-set-during-setup",
        "password": "will-be-set-during-setup",
        "hostname": "alraune"
    },
    "upgrade": {
        "url": "my-upgrade-server",
        "path": "/alraune/files/",
        "user": "alraune",
        "password": "my-password"
    },
    "log": {
        "url": "https://my-logstash-server",
        "user": "alraune",
        "password": "my-password"
    },
    "birthday": {
        "day": 22,
        "month": 11
    },
    "alerting": {
        "fromHour": 8,
        "toHour": 22
    },
    "sensorThreshold": 380,
    "volume": 20    
 }
 */
void writeConfigToFs(){
  Serial.println(F("Open configuration file from FS for update. This process deletes the current content"));
  File configFile = LittleFS.open("/config.json", "w+");
  if (configFile){
    JSONVar configObject;
    writeHomepageConfig(configObject); 
    writeWlanConfig(configObject);
    writeUpgradeConfig(configObject);
    writeLogConfig(configObject);
    writeBirthdayConfig(configObject);
    writeAlertingConfig(configObject);
    writeSensorConfig(configObject);
    writePlayerConfig(configObject);
   
    Serial.println(F("Configuration object created. Write to file"));
    String jsonString = JSON.stringify(configObject);    
    configFile.print(JSON.stringify(configObject));    
    Serial.println(F("Configuration to FS written"));
    configFile.close();
    
    Serial.print(F("Current configuration as JSON object: "));
    if (HIDE_PASSWORDS){
      jsonString.replace(wlanPassword,"*****");
      jsonString.replace(upgradePassword,"*****");
      jsonString.replace(logPassword,"*****");
    }
    Serial.println(jsonString);
  } else {
    Serial.println(F("Error loading config file!"));
  }
}

/*
 * Stores the name of the homepage in the given JSON
 * 
 * Example:
   "name": "will-be-set-during-setup"
 */
void writeHomepageConfig(JSONVar &configObject){ 
    configObject["name"] = pagename;
    Serial.print(F("Name: "));
    Serial.println(pagename);
}

/*
 * Stores the WLAN configuration in the given JSON
 * 
 * Example:
 "wlan": {
    "ssid": "will-be-set-during-setup",
    "password": "will-be-set-during-setup",
    "hostname": "alraune"
 }
 */
void writeWlanConfig(JSONVar &configObject){
  JSONVar wlan;

  wlan["ssid"] = ssid;
  Serial.print(F("SSID: "));
  Serial.println(ssid);
  
  wlan["password"] = wlanPassword;
  Serial.print(F("WLAN Password: "));
  if (HIDE_PASSWORDS){
    Serial.println(F("*****"));
  } else {
    Serial.println(wlanPassword);
  }
  
  wlan["hostname"] = hostname;
  Serial.print(F("Hostname: "));
  Serial.println(hostname);
    
  configObject["wlan"] = wlan;
}

/*
 * Stores the upgrade configuration in the given JSON
 * 
 * Example:
 "upgrade": {
    "url": "my-upgrade-server",
    "path": "/alraune/files/",
    "user": "alraune",
    "password": "my-password"
 }
 */
void writeUpgradeConfig(JSONVar &configObject){
  JSONVar upgrade;
  
  upgrade["url"] = upgradeUrl;
  Serial.print(F("Upgrade URL: "));
  Serial.println(upgradeUrl);

  upgrade["path"] = upgradePath;
  Serial.print(F("Upgrade Path: "));
  Serial.println(upgradePath);

  upgrade["user"] = upgradeUser;
  Serial.print(F("Upgrade User: "));
  Serial.println(upgradeUser);
  
  upgrade["password"] = upgradePassword;
  Serial.print(F("Upgrade Password: "));
  if (HIDE_PASSWORDS){
    Serial.println(F("*****"));
  } else {
    Serial.println(upgradePassword);
  }

  configObject["upgrade"] = upgrade;
}

/*
 * Stores the log configuration in the given JSON
 * 
 * Example:
 "log": {
    "url": "https://my-logstash-server",
    "user": "alraune",
    "password": "my-password"
 }
 */
void writeLogConfig(JSONVar &configObject){
  JSONVar log = configObject["log"];

  log["url"] = logUrl;
  Serial.print(F("Log URL: "));
  Serial.println(logUrl);

  log["user"] = logUser;
  Serial.print(F("Log User: "));
  Serial.println(logUser);

  log["password"] = logPassword;
  Serial.print(F("Upgrade Password: "));
  if (HIDE_PASSWORDS){
    Serial.println(F("*****"));
  } else {
    Serial.println(logPassword);
  }
    
  configObject["log"] = log;
}

/*
 * Stores the birthday configuration in the given JSON
 * 
 * Example:
 "birthday": {
    "day": 22,
    "month": 11
 }
 */
void writeBirthdayConfig(JSONVar &configObject){
  JSONVar birthdayObject;
    
  birthdayObject["day"] = birthdayDay;
  Serial.print(F("Day of Birthday: "));
  Serial.println(birthdayDay);
  
  birthdayObject["month"] = birthdayMonth;
  Serial.print(F("Month of Birthday: "));
  Serial.println(birthdayDay);
      
  configObject["birthday"] = birthdayObject;
}

/*
 * Stores the alerting configuration in the given JSON
 * 
 * Example:
 "alerting": {
    "fromHour": 8,
    "toHour": 22
 }
 */
void writeAlertingConfig(JSONVar &configObject){
  JSONVar alerting;
    
  alerting["fromHour"] = fromHour;
  Serial.print(F("Alert from hour: "));
  Serial.println(fromHour);

  alerting["toHour"] = toHour;
  Serial.print(F("Alert to hour: "));
  Serial.println(toHour);
  
  configObject["alerting"] = alerting;
}

/*
 * Stores the sensor configuration in the given JSON
 * 
 * Example:
 "sensorThreshold": 380
 */
void writeSensorConfig(JSONVar &configObject){
  configObject["sensorThreshold"] = sensorThreshold;
  Serial.print(F("Sensor Treshold: "));
  Serial.println(sensorThreshold);
}

/*
 * Stores the volume configuration in the given JSON
 * 
 * Example:
 "volume": 20
 */
void writePlayerConfig(JSONVar &configObject){
  configObject["volume"] = volume;
  Serial.print(F("Volume: "));
  Serial.println(volume);
}
