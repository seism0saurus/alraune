/*
 * Pump control with moisture sensor.
 * Needs
 * - a moisture sensor with max 3V output on the data channel
 * - a relay for 5V 
 * - a water pump for 5V
 * - and a DFPlayer mini
*/


/*
 * Inclusion of libraries
 * 
 * We need ESP8266WiFi for wireless lan connectivity
 * ESP8266mDNS is used for broadcasting of the domain alraune.local
 * ESP8266WebServer is our webserver, that handles the homepage, json output of the status and parses commands from the setuppage or homepage
 * CertStoreBearSSL is used for secure connection to servers and validation of upgrades
 * ESP8266HTTPClient is used to send log messages to a logserver
 * FS and LittleFS are used for storage the images, config files and the homepage files
 * time and millisDelay is for asynchronous handling of timeouts for the pump
 * SoftwareSerial and DFRobotDFPlayerMini are used for the audio playback
 * ESP8266httpUpdate is used for OTA upgrades of the system
 * Arduino_JSON is used for parsing or writing structured data like the config file or POST requests
 */
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <CertStoreBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <LittleFS.h>
#include <time.h>
#include <millisDelay.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <ESP8266httpUpdate.h>
#include <Arduino_JSON.h>

/* 
 * Hide passwords in log lines
 * Set to false for debugging of connection problems
 */
const boolean hidePasswords = true;

/* 
 * Upgrade configuration
 * 
 * The version is used to request upgrades from the upgrade server. If a newer version is available, the upgrade is send to the device and installed after verification
 * The upgrade URL is the hostname of the upgrade server. This is configured in the config.json. This system requires it to have a TLS secured http server on port 443
 * The upgrade path is the path to the project folder on the upgade server. On my upgrade server this is /alraune/files/
 * A complete request for the new firmare could be https://my-upgrade-server.org/alraune/files/firmware.bin
 * The upgrade user and password are needed for the basic authentication against the upgrade server. Since the firmware could contain sensitive information like credentials, they should be protected
 */
#ifndef VERSION
  #define VERSION "0.9.10"
#endif
String version = VERSION;
String upgradeUrl;
String upgradePath;
String upgradeUser;
String upgradePassword;

/* 
 * Code signing
 * 
 * The Alraune uses automatic code signing and verification with the Arduiono IDE
 * See https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#automatic-signing-only-available-on-linux-and-mac
 * If you want to sign the files manually or you don't have access to a good operating system, you can enable the manual mode.
 * Set MANUAL_SIGNING to 1 and provide the public key on the LittleFS
 */
#ifndef MANUAL_SIGNING
  #define MANUAL_SIGNING 0
#endif
#if MANUAL_SIGNING
  BearSSL::PublicKey *signPubKey = nullptr;
  BearSSL::HashSHA256 *hash;
  BearSSL::SigningVerifier *sign;
#endif

/*
 * WLAN configuration
 * 
 * Stores the SSID and the password to access the network
 * Both settings are read from the config.json from the LittleFS
 * After the startup the Alraune tries to connect to the configured WLAN
 * If the connection succeeds, the connectedToSsid is set to true and the complete system is started
 * If the connection fails, the connectedToSsid is set to false and only the needed parts are started to provide a setup page in the alraune WLAN under http://192.168.4.1/
 * The string wlanListJson will contain the JSON of the found wireless networks
 */
String ssid;
String wlanPassword;
boolean connectedToSsid = false;
String wlanListJson;

/*
 * HTTP client
 * 
 * Uses the secure HTTP client by BearSSL. It can access TLS encrypted webservers
 * The ca certificate as trust ancor is read from the LittleFS file ca.cer
 */
BearSSL::WiFiClientSecure client;

/*
 * Filesystem configuration
 * 
 * Configure a LittleFS filesystem to read and store config files and ressources for the webserver
 */
FS* fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
boolean fsOK;

/*
 * Media player configuration
 * 
 * Configure a serial interface on the pins 4 and 5 and initialize the conncted DFPlayer
 * The folder variables define wich folder number on the DFPlayer MiniSD card should be used for which purpose
 * They are important for the order in which you store the files on the card. See https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299#target_6
 * - waterRefillFolder contains the sounds, that should remind the owner to refill the water. The number of this sounds will be stored in waterRefillSounds
 * - birthdayFolder contains the sounds, that are played on the configured birthday of the owner. The number of this sounds will be stored in birthdaySounds
 * - christmasFolder contains the sounds, that are played on christmas eve (24.12). The number of this sounds will be stored in christmasSounds
 */
SoftwareSerial audioSerial(D2, D1); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
uint8_t volume = 10;
const uint8_t waterRefillFolder = 1;
const uint8_t birthdayFolder = 2;
const uint8_t christmasFolder = 3;
int waterRefillSounds = 0;
int birthdaySounds = 0;
int christmasSounds = 0;

/* 
 *  Webserver configuration
 *  
 *  Configure an ESP webserver on port 80
 *  The ip will be stored in the ip variable
 *  The hostname will be read from the config.json file and defines name of the webpage. The suffix .local will be appended since we use mDNS
 *  The pagename will be read from the config.json and is configured during setup. This will be the title of the homepage shown in the browser
 */
const int port = 80;
ESP8266WebServer server(port);
String ip;
String hostname;
String pagename;

/*
 * Pin configuration
 * 
 * Configure which pins are used for the sensor and actuator as well as the led
 * The sensorPin is connected to the moisture sensor and should not get more than 1V (pure ESP) or 3V (D1 Mini)
 * The relayPin is the pin that controls the realay to power the water pump
 * The led is used to signal access to the webserver, pump activation or upgrades
 */
const int sensorPin = A0;
const int releayPin = D0;
const int led = LED_BUILTIN;

/* 
 *  Logging configuration
 *  
 *  Configure the URL and credentials for the logserver
 *  The logUrl is the complete URL that can consume json log events like logstash. This can be configured through config.json
 *  The variables logUser and logPassword contain the credentials to the logserver
 *  Since you can write events to the server, it should be protected by TLS and basic auth. These are configured through config.json
 */
String logUrl;
String logUser;
String logPassword;


/* 
 * Business variables
 * 
 * Variables that are actually used for calculations of water usage, moisture and activation of the water pump
 * The sensorThreshold will be overridden with the value from config.json and can be recalibrated on the homepage
 * The sensorValue will store the actual value of the moisture sensor
 * The variable needsWater is set to true, if the earth is too dry. Otherwise it is false
 * The variable needsRefill is set to true, if the water container needs to be refilled. Otherwise it is false
 * The variables birthdayDay and birthdayMonth store the birthday of the owner. They will be configured during the setup process and stored in the config.json. The Alraune will sing for the owner on his or her birthday
 * The variables christmasDay and christmasMonth store the day, when the Alraune will sing christmas songs or say nice things to the owner
 * The variables fromHour and toHour sotre the time range, in wich the Alraune will make sounds. It will stay silent in the hours outside that range
 */
int sensorThreshold = 380;
int sensorValue = 0;
boolean needsWater = false;
boolean needsRefill = false;
uint8_t birthdayDay = 22;
uint8_t birthdayMonth = 11;
const uint8_t christmasDay = 24;
const uint8_t christmasMonth = 12;
uint8_t fromHour = 8;
uint8_t toHour = 22;

/* 
 * Delays
 * 
 * The delays are used to run functions after a fixed time or regularly
 * The pumpDelay is started, when the moisture of the earth is too low. After 10 seconds the pump is stopped again
 * The moistureDelay is started at the same time as the pumpDelay but runs one hour. If the earth is still to dry after that time, we can assume, that there was not enough water in the water container and we inform the owner
 * The networkMonitoringDelay ist started, when a network error occurs. After 10 seconds we check the connectivity to the WLAN and restart the network, if we still don't have a connection
 * The logDelay is used to send a statistic log event to the logserver every minute
 * The easterEggDelay runs every hour and plays a birthday or christmas sound, if it is the correct date and time
 * The upgradeDelay is used to check for upgrades every 10 minutes
 */
millisDelay pumpDelay;
millisDelay moistureDelay;
millisDelay networkMonitoringDelay;
millisDelay logDelay;
millisDelay easterEggDelay;
millisDelay upgradeDelay;

/*
 * Setup method to prepare the Alraune
 * 
 * After an inial delay, the pins and the serial interface of the D1 mini are initialized. After this stage we can see log messages through the serial interface
 * The next stage initializes the filesystem LittleFS, since we need the config file and the certificate for the rest of the setup
 * Now we can load the configuration from the config.json file.
 * 
 * After we loaded the config we can try to setup the network. First the Alraune tries to connect to the WLAN from the config.json.
 * If it fails to connect multuple times it opens an Access Point with a WLAN called alraune and listens on the IP 192.168.4.1
 * If the connection to the configured network works, no Acces Point is started and the Alraune gets an IP from the DHCP server and announces its name alraune.local through mDNS
 * After the network is running in one of the two modes, the HTTP server is started
 * The server will deliver either the setup page, if it runs in AP mode, or the homepage of the Alraune.
 * To finish the setup got to the WLAN alraune and open http://192.168.4.1 in your browser
 * 
 * Only if the Alraune could connect to your configured WLAN it starts the other features
 * 
 * The logging is responsible for sending regular statistics to the logserver, where the data can be visualized and processed for alarms or other stuff
 * Furthermore it can send errors or important logs to the logserver
 * The certiicate of the CA of the logserver has to be saved as ca.cer in the LittleFS. The let's encrypt certificate is preconfigured 
 * 
 * After the logging is enabled, the DFPlayer mini is initialized through the software serial
 * The volume is set to the configured value and the number of sounds is determined. This is later used to play a random sound
 * 
 * After that the clock is synchronized with a time server over NTP. The correct time is needed for verification of TLS certificates or other signatures
 * Furthermore we need the date to check the birthday and christmas easter eggs
 *  
 * When the clock is synchronized the easter eggs can be handled. On the birthday of the owner an on christmas eve the Alraune will play some special sounds
 * 
 * At last the upgrade delay is started. Every 10 minutes the Alraune will connect to the configured upgrade server and sends the current version
 * If a newer versino is available the files on the LittleFS and the firmware will be updated. After that the system automatically reboots
 */
void setup() {
  delay(1000);
  setupPins();
  setupSerial();
  setupFs();
  loadConfigFromFs();
  setupNetwork();
  setupServer();
  if (connectedToSsid){
    setupLogging();
    setupDFPlayer();
    setupClock();
    setupEasterEggs();
    setupUpgrade();
    Serial.println(F("Finished setup. Please go to http://alraune.local to view the dashboard"));
  } else {
    Serial.println(F("Finished setup. Please connect to 'alraune' WLAN with password 'WaterIsLive' and go to http://alraune.local to finish configuration"));
  }
}


/*
 * The loop method for the execution of the application
 * 
 * When the setup is not finished and therefore there is no network connectivity, the Alraune will only serve the setup page
 * After the setup is finished AND eherefore the Alraune is in your wireless network, the homepage is served and all the features are handled
 * 
 * The alraune will publish it's name alraune.local through mDNS
 * 
 * The moisture is checked and the pump will be activated, if needed
 * 
 * If the pump was active and the earth is still too dry, the owner will be informed via sounds by the Alraune
 * 
 * The media player handler will get the status from the DFPlayer and print error messages
 * 
 * The eeaster egg handler will play a special sound every hour during birthdays and christmas eve
 * 
 * The upgrade function checks if a newer version is available on the upgrade server and perfomse the upgrade
 * 
 * The logger sends the current status to the log server for further processing
 * 
 * If any network errors were detected, the network is reconnected by the network error handler
 */
void loop() { 
  handleHttpRequests();
  updateDns();
  if (connectedToSsid){
    activatePumpIfPlantIsDry();
    warnHumanIfPlantisStillDry();
    handleMediaPlayer();
    handleEasterEggs();
    upgrade();
    logStatus();
    handleNetworErrorsIfNeeded();
  }
}
