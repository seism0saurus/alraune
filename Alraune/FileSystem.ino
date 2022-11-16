/*
 * Start the filesystem
 * 
 * A LittleFS Filesystem is started and the status is checked
 * The status will be stored in the global varaiable fsOK, so we can check it later again
 */
void setupFs(){
  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  if (fsOK) {
    Serial.println(F("FS successfully started"));
  } else {
    Serial.println(F("FS has an error!"));
  }
}
