/*
 * Setup easter eggs. Play a special sound every hour on special occasions
 * 
 * These are the birthday of the owner and the christmas eve
 */
void setupEasterEggs(){
  easterEggDelay.start(1000*60*60*1);
}

/*
 * Compates date and hour and play an easter egg sound
 * 
 * The special days are the birthday of the owner and the christmas eve
 * 
 */
void handleEasterEggs(){
  if (easterEggDelay.justFinished()){
    easterEggDelay.repeat();
    
    if ( isBirthday() && isInHours() ){
      playBirthdaySound();
    }

    if ( isChristmas() && isInHours() ){
      playChristmasSound();
    }
  } else {
    delay(1);
  }
}
