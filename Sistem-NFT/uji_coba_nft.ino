#include <FS.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// RTC tanpa module
#include <TimeLib.h>                                    
#include <WidgetRTC.h>  
//sensor ds18b20
#include <DallasTemperature.h>
#include <OneWire.h>
#define sensords18b20 2 //D4
//sensor dht11
#include <dht11.h>
#define sensordht11 14 //D5
//relay
#define pumprelay 15 //D8 untuk pompa mist
#define aerator_relay 13 //D7
#define pinrelay3 12 //D6

//deklarasi variabel
//sensor ds18b20
OneWire oneWire(sensords18b20);
DallasTemperature sensor(&oneWire);
float temperatur_cairan;
//sensor dht11
float humidity, temperatur_lingkungan;
dht11 DHT11;
//RTC 
BlynkTimer timer;                                       
WidgetRTC rtc; 
unsigned long startMillis;                              
unsigned long currentMillis;                            
const unsigned long period = 1000; 


//wifi
char auth[] = "nOzbn3Y2a-JLkR6wc4XoqocjAMgiLJtB";
char ssid[] = "Fuh";
char pass[] = "kvev1889";

//char blynk_token [34] ="nOzbn3Y2a-JLkR6wc4XoqocjAMgiLJtB";

bool shouldSaveConfig = false;

//relay
void pumprelayON(){
  digitalWrite(pumprelay, LOW);
  //Blynk.virtualWrite(V5, 255);
}
void pumprelayOFF(){
  digitalWrite(pumprelay, HIGH);
  //Blynk.virtualWrite(V5, 0);
}

void aerator_relayON(){
  digitalWrite(aerator_relay, LOW);
}
void aerator_relayOFF(){
  digitalWrite(aerator_relay, HIGH);
}

//sensor ds18b20
void send_ds18b20(){
  sensor.requestTemperatures();
  temperatur_cairan =sensor.getTempCByIndex(00);
  Serial.println("Celcius temperature:");
  Serial.println(temperatur_cairan);
  Blynk.virtualWrite(V1, temperatur_cairan);
  if(digitalRead(aerator_relay)==1){
    if (hour()>= 11 && hour()<=14){
      if (temperatur_cairan > 25){
        aerator_relayON();
      }
      else if (temperatur_cairan <= 25){
        aerator_relayOFF();
      }
    }
  }
  Serial.println(digitalRead(aerator_relay));
}
//sensor dht11
void send_dht11(){
  int chk = DHT11.read(sensordht11);
  humidity = DHT11.humidity;
  temperatur_lingkungan = DHT11.temperature;
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print(" Temperatur lingkungan:" );
  Serial.println(temperatur_lingkungan);
  Blynk.virtualWrite(V2, temperatur_lingkungan);
  Blynk.virtualWrite(V3, humidity); 
  if(digitalRead(pumprelay) == 1){
    if(minute()>= 0 && minute()<= 10){
      if (humidity < 60){
        pumprelayON();
        delay (1000);
        pumprelayOFF();
      }
      else if (humidity >= 60){
        pumprelayOFF();
      }
    }
    else if( minute()>=30 && minute()<=40){
     if (humidity < 60){
        pumprelayON();
        delay (1000);
        pumprelayOFF();
      }
      else if (humidity >= 60){
        pumprelayOFF();
     } 
    }
  Serial.println(digitalRead(pumprelay));
}
}



 
// RTC
BLYNK_CONNECTED()                                               
{ 
  rtc.begin(); 
  }
void sendRTC(){
  currentMillis = millis();                                                       
  if(currentMillis - startMillis > period)                                        
  {
    String currentTime = String(hour()) + ":" + minute() + ":" + second();        
    String currentDate = String(day()) + " " + month() + " " + year();            
    Serial.print("Current time: ");                                               
    Serial.print(currentTime);
    Serial.print(" ");
    Serial.print(currentDate);
    Serial.println();

    Blynk.virtualWrite(V7, currentTime);                                          
    Blynk.virtualWrite(V8, currentDate);                                          
    int getSecond = second();                                                     
    if (getSecond > 30)
    { digitalWrite(16,HIGH);}                                                      
    if (getSecond < 30)
    { digitalWrite(16,LOW); }                                                                                                       
    startMillis = millis();                                                      
  }
}

void setup (){
  //initialize();
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  sensor.begin();
  digitalWrite(pumprelay, HIGH);
  digitalWrite(aerator_relay, HIGH);
  //Blynk.begin(blynk_token, WiFi.SSID().c_str(), WiFi.psk().c_str());
  timer.setInterval(1000L, send_ds18b20); 
  timer.setInterval(1000L, send_dht11);
  timer.setInterval(1000L, sendRTC);
 
  
 
}
//int lastConnectionAttempt = millis();
//int connectionDelay = 5000; // try to reconnect every 5 seconds
void loop (){
  Blynk.run();
  timer.run();

}



  
