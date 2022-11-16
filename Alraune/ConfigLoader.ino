/*
 * Loads the config.json from the LittleFS filesystem
 * 
 * This overrides all currently configured values with the variables from the config.json
 * If the file could not be found or loaded, an error is printed in the serial interface
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
void loadConfigFromFs(){
  Serial.println(F("Load configuration file from FS"));
  File configFile = LittleFS.open("/config.json", "r");
  if (configFile){
    String configString = configFile.readString();
    JSONVar configObject = JSON.parse(configString);
    
    if (JSON.typeof(configObject) == "undefined") {
      Serial.println(F("Parsing config file failed!"));
      return;
    }

    loadHomepageConfig(configObject); 
    loadWlanConfig(configObject);
    loadUpgradeConfig(configObject);
    loadLogConfig(configObject);
    loadBirthdayConfig(configObject);
    loadAlertingConfig(configObject);
    loadSensorConfig(configObject);
    loadPlayerConfig(configObject);
    Serial.println(F("Configuration from FS loaded"));

    configFile.close();
  } else {
    Serial.println(F("Error loading config file!"));
  }
}

/*
 * Loads the name of the homepage from the given JSON
 * 
 * If the given JSON does not have a property "name", an error is printed in the serial interface
 * 
 * Example:
   "name": "will-be-set-during-setup"
 */
void loadHomepageConfig(JSONVar &configObject){
  if (configObject.hasOwnProperty("name")){   
    pagename = (const char*)configObject["name"];
    Serial.print(F("Name: "));
    Serial.println(pagename);
  } else {
    Serial.println(F("ERROR: Could not read name object in the config file!"));
  }
}

/*
 * Loads the WLAN configuration from the given JSON
 * 
 * If the given JSON does not have a property "wlan" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "wlan": {
    "ssid": "will-be-set-during-setup",
    "password": "will-be-set-during-setup",
    "hostname": "alraune"
 }
 */
void loadWlanConfig(JSONVar &configObject){
  if (configObject.hasOwnProperty("wlan")){
    JSONVar wlan = configObject["wlan"];
    
    if (wlan.hasOwnProperty("ssid")){ 
      ssid = (const char*)wlan["ssid"];
      Serial.print(F("SSID: "));
      Serial.println(ssid);
    } else {
      Serial.println(F("ERROR: Could not read ssid from wlan object in the config file!"));
    }
    
    if (wlan.hasOwnProperty("password")){ 
      wlanPassword = (const char*)wlan["password"];
      Serial.print(F("WLAN Password: "));
      if (hidePasswords){
        Serial.println(F("*****"));
      } else {
        Serial.println(wlanPassword);
      }
    } else {
      Serial.println(F("ERROR: Could not read password from wlan object in the config file!"));
    }
    
    if (wlan.hasOwnProperty("hostname")){ 
    hostname = (const char*)wlan["hostname"];
    Serial.print(F("Hostname: "));
    Serial.println(hostname);
    } else {
      Serial.println(F("ERROR: Could not read password from wlan object in the config file!"));
    }
  } else {
    Serial.println(F("ERROR: Could not read name from config file!"));
  }
}

/*
 * Loads the upgrade configuration from the given JSON
 * 
 * If the given JSON does not have a property "upgrade" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "upgrade": {
    "url": "my-upgrade-server",
    "path": "/alraune/files/",
    "user": "alraune",
    "password": "my-password"
 }
 */
void loadUpgradeConfig(JSONVar &configObject){
  if (configObject.hasOwnProperty("upgrade")){
    JSONVar upgrade = configObject["upgrade"];
  
    if (upgrade.hasOwnProperty("url")){
      upgradeUrl = (const char*)upgrade["url"];
      Serial.print(F("Upgrade URL: "));
      Serial.println(upgradeUrl);
    } else {
      Serial.println(F("ERROR: Could not read url from upgrade object in the config file!"));
    }  
  
    if (upgrade.hasOwnProperty("path")){
      upgradePath = (const char*)upgrade["path"];
      Serial.print(F("Upgrade Path: "));
      Serial.println(upgradePath);
    } else {
      Serial.println(F("ERROR: Could not read path from upgrade object in the config file!"));
    }  
    
    if (upgrade.hasOwnProperty("user")){
      upgradeUser = (const char*)upgrade["user"];
      Serial.print(F("Upgrade User: "));
      Serial.println(upgradeUser);
    } else {
      Serial.println(F("ERROR: Could not read user from upgrade object in the config file!"));
    }
    
    if (upgrade.hasOwnProperty("password")){
      upgradePassword = (const char*)upgrade["password"];
      Serial.print(F("Upgrade Password: "));
      if (hidePasswords){
        Serial.println(F("*****"));
      } else {
        Serial.println(upgradePassword);
      }
    } else {
      Serial.println(F("ERROR: Could not read password from upgrade object in the config file!"));
    }
  } else {
    Serial.println(F("ERROR: Could not read upgrade object from config file!"));
  }
}

/*
 * Loads the log configuration from the given JSON
 * 
 * If the given JSON does not have a property "log" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "log": {
    "url": "https://my-logstash-server",
    "user": "alraune",
    "password": "my-password"
 }
 */
void loadLogConfig(JSONVar &configObject){
  if (configObject.hasOwnProperty("log")){
    JSONVar log = configObject["log"];
  
    if (log.hasOwnProperty("url")){
      logUrl = (const char*)log["url"];
      Serial.print(F("Log URL: "));
      Serial.println(logUrl);
    } else {
      Serial.println(F("ERROR: Could not read url from log object in the config file!"));
    }

    if (log.hasOwnProperty("user")){
      logUser = (const char*)log["user"];
      Serial.print(F("Log User: "));
      Serial.println(logUser);
    } else {
      Serial.println(F("ERROR: Could not read user from log object in the config file!"));
    }

    if (log.hasOwnProperty("password")){
      logPassword = (const char*)log["password"];
      Serial.print(F("Upgrade Password: "));
      if (hidePasswords){
        Serial.println(F("*****"));
      } else {
        Serial.println(logPassword);
      }
    } else {
      Serial.println(F("ERROR: Could not read password from log object in the config file!"));
    }
  } else {
    Serial.println(F("ERROR: Could not read log object from config file!"));
  }
}

/*
 * Loads the birthday configuration from the given JSON
 * 
 * If the given JSON does not have a property "birthday" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "birthday": {
    "day": 22,
    "month": 11
 }
 */
void loadBirthdayConfig(JSONVar &configObject){
  if (configObject.hasOwnProperty("birthday")){  
    JSONVar birthdayObject = configObject["birthday"];
    
    if (birthdayObject.hasOwnProperty("day")){ 
      birthdayDay = (int)birthdayObject["day"];
      Serial.print(F("Day of Birthday: "));
      Serial.println(birthdayDay);
    } else {
      Serial.println(F("ERROR: Could not read day from birthday object in the config file!"));
    }
    
    if (birthdayObject.hasOwnProperty("month")){
      birthdayMonth = (int)birthdayObject["month"];
      Serial.print(F("Month of Birthday: "));
      Serial.println(birthdayMonth);
    } else {
      Serial.println(F("ERROR: Could not read month from birthday object in the config file!"));
    }
  } else {
    Serial.println(F("ERROR: Could not read birthday object from config file!"));
  }
}

/*
 * Loads the alerting configuration from the given JSON
 * 
 * If the given JSON does not have a property "alerting" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "alerting": {
    "fromHour": 8,
    "toHour": 22
 }
 */
void loadAlertingConfig(JSONVar &configObject){
  if (configObject.hasOwnProperty("alerting")){  
    JSONVar alerting = configObject["alerting"];
    
    if (alerting.hasOwnProperty("fromHour")){   
      fromHour = (int) alerting["fromHour"];
      Serial.print(F("Alert from hour: "));
      Serial.println(fromHour);
    } else {
      Serial.println(F("ERROR: Could not read fromHour from alerting object in the config file!"));
    }
    
    if (alerting.hasOwnProperty("toHour")){  
      toHour = (int)alerting["toHour"];
      Serial.print(F("Alert to hour: "));
      Serial.println(toHour);
    } else {
      Serial.println(F("ERROR: Could not read toHour from alerting object in the config file!"));
    }
  } else {
    Serial.println(F("ERROR: Could not read alerting object from config file!"));
  }
}

/*
 * Loads the sensor configuration from the given JSON
 * 
 * If the given JSON does not have a property "sensorThreshold" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "sensorThreshold": 380
 */
void loadSensorConfig(JSONVar &configObject){
    if (configObject.hasOwnProperty("sensorThreshold")){   
      sensorThreshold= (int)configObject["sensorThreshold"];
      Serial.print(F("Sensor Treshold: "));
      Serial.println(sensorThreshold);
    } else {
      Serial.println(F("ERROR: Could not read sensorThreshold object in the config file!"));
    }
}

/*
 * Loads the volume configuration from the given JSON
 * 
 * If the given JSON does not have a property "volume" or it does not contain all needed information, an error is printed in the serial interface
 * 
 * Example:
 "volume": 20
 */
void loadPlayerConfig(JSONVar &configObject){
    if (configObject.hasOwnProperty("volume")){   
      volume= (int)configObject["volume"];
      Serial.print(F("Volume: "));
      Serial.println(volume);
    } else {
      Serial.println(F("ERROR: Could not read volume object in the config file!"));
    }
}
