void saveConfigCallback () {
  Serial.println(F("Should save config"));
  shouldSaveConfig = true;
}

void initSPIFFS() {
  //read token from FS
  Serial.println(F("mounting FS..."));
  if (SPIFFS.begin()) {
    Serial.println(F("mounted file system"));
    if (SPIFFS.exists("/token.txt")) {
      //file exists, reading and loading
      Serial.println(F("reading config file"));
      File configFile = SPIFFS.open("/token.txt", "r");
      if (configFile) {
        Serial.print(F("opened config file")); 
        while(configFile.available()) 
        {
          //read line by line from the file
          String line = configFile.readStringUntil('\n');
          strcpy(blynk_token, line.c_str());
          Serial.println(F(" success"));
        } 
      } else {
         Serial.println(F("failed to load token"));
      }

      configFile.close();
    }
  }
   else {
    Serial.println(F("failed to mount FS"));
  }
}

void saveToken() {
    Serial.println("saving config");
    File configFile = SPIFFS.open("/token.txt", "w");
    if (!configFile) 
    {
      Serial.println("file creation failed");
    } else {
      Serial.println("File Created!");
      configFile.println(blynk_token);    
    }
    configFile.close();
}
WiFiManager wifiManager;
void initialize(){
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  

  //clean FS, for testing
  SPIFFS.format();


  initSPIFFS();

  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);

  if (!wifiManager.autoConnect("wifiOP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println("connected...yeey :)");
  //wifiManager.resetSettings();

  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    saveToken();
  }
  

}
