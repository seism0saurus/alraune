/*
 * Setup the clock and sync it via Network Time Procol
 * 
 * The correct time is needed for TLS handshakes and easter eggs
 */
void setupClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println(F("Waiting for NTP time sync"));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.println(F("Waiting for NTP time sync"));
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  String message = "Current time is ";
  message += asctime(&timeinfo);
  log(message);
}

/*
 * Provides the current time as tm struct
 */
tm getTimeInfo(){
    struct tm timeinfo;
    time_t now = time(nullptr);
    gmtime_r(&now, &timeinfo);

    String message = "Current time is ";
    message += asctime(&timeinfo);
    log(message);
    
    return timeinfo;
}

/*
 * Checks if the current date is the configured birthday
 * 
 * Returns true if it is the owners birthday
 * Returns false otherwise
 */
boolean isBirthday(){
    struct tm timeinfo = getTimeInfo();
 
    const uint8_t day = timeinfo.tm_mday;
    const uint8_t month = timeinfo.tm_mon + 1;

    return ( birthdayMonth == month && birthdayDay == day );
}

/*
 * Checks if the current date is christmas eve
 * 
 * Returns true if it is christmas eve
 * Returns false otherwise
 */
boolean isChristmas(){
    struct tm timeinfo = getTimeInfo();
  
    const uint8_t day = timeinfo.tm_mday;
    const uint8_t month = timeinfo.tm_mon + 1;

    return ( christmasMonth == month && christmasDay == day );
}

/*
 * Checks if the current time is within the configured time range for sounds
 * 
 * Returns true if the current time is within the time range
 * Returns false otherwise
 */
boolean isInHours(){
    struct tm timeinfo = getTimeInfo();
   
    const uint8_t hour = timeinfo.tm_hour;    
    
    return ( hour >= fromHour && hour <= toHour );
}
