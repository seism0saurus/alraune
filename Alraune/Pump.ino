/*
 * Activate the water pump if needed
 * 
 * This method checks the current moisture value from the external sensor and starts the pump, if it is too dry
 * Delays are used to control the duration of the pumping and when the next test should be made
 */
void activatePumpIfPlantIsDry(){
  // The sensor ranges from 0-1000. The value is higher, if the soil is more dry. Invert the value, since we want to measure moisture not dryness
  sensorValue = 1000 - analogRead(sensorPin);
  
  if (sensorValue < sensorThreshold){
    needsWater = true;
    if (!pumpDelay.isRunning() && !moistureDelay.isRunning()){
      Serial.println(F("Plant needs water. Starting pump"));
      digitalWrite(releayPin, LOW);
      pumpDelay.start(10000);
      moistureDelay.start(3600000);
    }
  } else {
    needsWater = false;
  }
  
  if (pumpDelay.justFinished()){
    Serial.println(F("Pump delay finished. Stopping pump"));
    digitalWrite(releayPin, HIGH);
  }
}

/*
 * Warn owner if the plant is too dry after the pump was active
 * 
 * Plays a random water refill sounds when the delay is finished and the earth is still to dry
 * We D1 Mini has only one analog input, so we can't use a second sensor to check, if there is still water in the water container
 * Therefore I assume that the moisture level will go up a lot, if water got pumped into the earth
 * If thats not the case, the water must be empty or the tube is no longer in the plant
 */
void warnHumanIfPlantisStillDry(){
  // The sensor ranges from 0-1000. The value is higher, if the soil is more dry. Invert the value, since we want to measure moisture not dryness
  sensorValue = 1000 - analogRead(sensorPin);

  // if the water still needs water, the water tank has to be refilled
  if (moistureDelay.justFinished()){
    if (sensorValue < sensorThreshold){
      Serial.println(F("Moisture delay finished. Planed still needs water. Human has to refill"));
      if (isInHours()){
        playWaterRefillSound();
      }
      needsRefill = true;
    } else {
      Serial.println(F("Moisture delay finished. Planed has enough water. Nothing to do"));
      needsRefill = false;
    }
  } else {
    delay(1);
  }
}
