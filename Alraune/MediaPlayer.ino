/*
 * Setup the DFPlayer
 * 
 * Start the player by initializing the serial interface from the D1 Mini to the DFPlayer with 9600 baud
 * The system will not reboot, if there is no acknowledgement by the player to prevent problems with picky components
 * 
 * Set the volume to the configured value
 * 
 * Read the folders for water refill sounds, birthday sounds and christmas sounds and store the number of sounds in the global variables
 */
void setupDFPlayer(){
  Serial.println(F("Start DFPlayer. This may take 3~5 seconds"));
  audioSerial.begin(9600);
    
  if (!myDFPlayer.begin(audioSerial, false)) {
      Serial.println(F("Error start DFPlayer! 1.Please recheck the connection! 2.Please insert the SD card!"));
      while(true);
  }
  Serial.println(F("DFPlayer succesfully started"));
  
  //Set volume value. From 0 to 30
  myDFPlayer.volume(volume);

  waterRefillSounds = myDFPlayer.readFileCountsInFolder(waterRefillFolder);
  String message = "Found ";
  message += waterRefillSounds;
  message += " Sounds in refill sound folder nr. ";
  message += waterRefillFolder;
  log(message);
  birthdaySounds = myDFPlayer.readFileCountsInFolder(birthdayFolder);
  message = "Found ";
  message += birthdaySounds;
  message += " Sounds in birthday sound folder nr. ";
  message += birthdayFolder;
  log(message);
  christmasSounds = myDFPlayer.readFileCountsInFolder(christmasFolder);
  message = "Found ";
  message += christmasSounds;
  message += " Sounds in christmas sound folder nr. ";
  message += christmasFolder;
  log(message);
}

/*
 * Checks if the player is available and reads the current status
 * 
 * The information is handled to the printDetailsForMediaPlayer function, which decodes the numbers
 */
void handleMediaPlayer(){
  if (myDFPlayer.available()) {
    printDetailsForMediaPlayer(myDFPlayer.readType(), myDFPlayer.read());
  }
}

/*
 * Plays a random water refill sound
 * 
 * The sound is choosen with a random number generator in the range from 1 to the found number of sounds in the folder
 * 
 * After the sound is played, the handleMediaPlayer function is called to get the current status of the player
 */
void playWaterRefillSound(){
   int song = random(1, waterRefillSounds);
   String message = "Play water refill sound ";
   message += song;
   message += " from ";
   message += waterRefillSounds;
   message += " available Sounds";
   Serial.println(message);
   myDFPlayer.playFolder(waterRefillFolder, song);
   handleMediaPlayer();
}

/*
 * Plays a random birthday sound
 * 
 * The sound is choosen with a random number generator in the range from 1 to the found number of sounds in the folder
 * 
 * After the sound is played, the handleMediaPlayer function is called to get the current status of the player
 */
void playBirthdaySound(){
   int song = random(1, birthdaySounds);
   String message = "Play birthday sound ";
   message += song;
   message += " from ";
   message += birthdaySounds;
   message += " available Sounds";
   Serial.println(message);
   myDFPlayer.playFolder(birthdayFolder, song);
   handleMediaPlayer();
}


/*
 * Plays a random christmas sound
 * 
 * The sound is choosen with a random number generator in the range from 1 to the found number of sounds in the folder
 * 
 * After the sound is played, the handleMediaPlayer function is called to get the current status of the player
 */
void playChristmasSound(){
   int song = random(1, christmasSounds);
   String message = "Play christmas sound ";
   message += song;
   message += " from ";
   message += christmasSounds;
   message += " available Sounds";
   Serial.println(message);
   myDFPlayer.playFolder(christmasFolder, song);
   handleMediaPlayer();
}


/*
 * Decode media player messages
 * 
 * Decodes the media player messages from the serial interface
 * They consist of a type and a value
 * Depending both values a message is logged to the serial interface
 * If it is an error, the error is logged to the logserver
 */
void printDetailsForMediaPlayer(uint8_t type, int value)
{
    switch (type) {
        case TimeOut:
            Serial.println(F("Mediaplayer: Time Out"));
            break;
        case WrongStack:
            Serial.println(F("Mediaplayer: Stack Wrong"));
            break;
        case DFPlayerCardInserted:
            Serial.println(F("Mediaplayer: Card Inserted"));
            break;
        case DFPlayerCardRemoved:
            Serial.println(F("Mediaplayer: Card Removed"));
            break;
        case DFPlayerCardOnline:
            Serial.println(F("Mediaplayer: Card Online"));
            break;
        case DFPlayerPlayFinished:
            Serial.println(F("Finished playing"));
            break;
        case DFPlayerError:
            switch (value) {
                case Busy:
                    Serial.println(F("Mediaplayer: Error: Card not found"));
                    break;
                case Sleeping:
                    Serial.println(F("Mediaplayer: Error: Sleeping"));
                    break;
                case SerialWrongStack:
                    Serial.println(F("Mediaplayer: Error: Get Wrong Stack"));
                    break;
                case CheckSumNotMatch:
                    Serial.println(F("Mediaplayer: Error: Check Sum Not Match"));
                    break;
                case FileIndexOut:
                    Serial.println(F("Mediaplayer: Error: File Index Out of Bound"));
                    break;
                case FileMismatch:
                    Serial.println(F("Mediaplayer: Error: Cannot Find File"));
                    break;
                case Advertise:
                    Serial.println(F("Mediaplayer: Error: In Advertise"));
                    break;
            }
            break;
    }
}
