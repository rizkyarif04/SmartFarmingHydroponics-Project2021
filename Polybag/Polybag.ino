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
#define sensords18b20 15 //D8
//sensor dht11
#include <dht11.h>
#define sensordht11 13 //D7
//sensor yl69
#define powerPin D2 //pengganti  vcc
//pompa
#define PIN_PUMP1 D5 //untuk pompa drip tetes
//define PIN_PUMP2 5 //D1
//Mist Aerator
#define PIN_MIST1 12 //D6 untuk pompa mist
//#define PIN_MIST2 15 //D5

//deklarasi variabel
//sensor ds18b20
OneWire oneWire(sensords18b20);
DallasTemperature sensor(&oneWire);
float temperatur_cairan;
//sensor dht11
float humidity, temperatur_lingkungan;
dht11 DHT11;
//sensor yl69
const int selectPins[3] = {D3, D2, D1};
const int analogInput = A0;

//RTC
BlynkTimer timer;                                       
WidgetRTC rtc; 
unsigned long startMillis;                              
unsigned long currentMillis;                            
const unsigned long period = 1000; 

//wifi
//char auth[] = "29u1yQZ3QEBzjdhXVsNtnedWzFgOTH42";
//char ssid[] = "vivo 1902";
//char pass[] = "uwu1234567";



char blynk_token [34] ="nOzbn3Y2a-JLkR6wc4XoqocjAMgiLJtB";

bool shouldSaveConfig = false;


//pompa drip tetes
void pump_relayON(){
  digitalWrite(PIN_PUMP1, LOW); 
}
void pump_relayOFF(){
  digitalWrite(PIN_PUMP1, HIGH);
}
//pompa mist
void mist_relayON(){
  digitalWrite(PIN_MIST1, LOW);
}
void mist_relayOFF(){
  digitalWrite(PIN_MIST1, HIGH);
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
  if(digitalRead(PIN_MIST1) == 1){
    if(minute()>= 0 && minute()<=5){
      if (humidity <= 40){
        mist_relayON();
        delay (1000);
        mist_relayOFF();
      }
      else if (humidity > 40){
        mist_relayOFF();
      }
    }
    else if (minute()>= 30 && minute()<= 35){
      if (humidity <= 40){
        mist_relayON();
        delay (1000);
        mist_relayOFF();
      }
      else if (humidity > 40){
        mist_relayOFF();
      }
    }
  }
  Serial.println(digitalRead(PIN_MIST1));
}

int numpin = -1;
//sensor yl69x`
void send_yl69(){
  digitalWrite(powerPin, HIGH);
  int inputValue = 1023 - analogRead(analogInput);
  int level_moisture = inputValue*100/1023;
  Serial.print("Kelembapan:");
  Serial.print(String(level_moisture) + "\t");
  digitalWrite(powerPin, LOW);
  Blynk.virtualWrite(V4, level_moisture);
  Serial.println();
  delay(1000);
  
  int readpin= digitalRead(PIN_PUMP1);
  numpin = numpin + readpin;
  readpin= readpin+1;
  if (numpin >= 0){
    if (readpin>=1){
     if (minute()>= 0 && minute()<= 5){
      if (level_moisture <= 70){
           pump_relayON();
           delay (1000);
        }
      else if (level_moisture >70){
          pump_relayOFF();
          numpin = -1;
        }
      }
     else if (minute()>=30 && minute()<=35){
      if (level_moisture <= 70){
           pump_relayON();
           delay (1000);
        }
      else if (level_moisture > 70){
          pump_relayOFF();
          numpin = -1;
        } 
      }
     else{
       //readpin = readpin-1;
       pump_relayOFF();
       numpin = -1;
     }
    }
  }

 
 Serial.println("Numpin="+String(numpin));
 Serial.println("dig Read="+String(digitalRead(PIN_PUMP1)));
  }
  



//RTC
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

void setup(){
  Serial.begin(9600);
  initialize();
  sensor.begin(); //setup ds18b20
  for (int i=0; i < 3; i++)
  {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], HIGH);
  }
  // jadikan pin power sebagai output
  pinMode(powerPin, OUTPUT);
  pinMode(PIN_PUMP1, OUTPUT);
  // default bernilai LOW
  digitalWrite(powerPin, LOW);
  Blynk.begin(blynk_token, WiFi.SSID().c_str(), WiFi.psk().c_str());
  digitalWrite(PIN_PUMP1, HIGH);
  digitalWrite(PIN_MIST1, HIGH);
  timer.setInterval(1000L, send_dht11);
  timer.setInterval(1000L, send_yl69);
  timer.setInterval(1000L, sendRTC); 

}

void loop ()
{
  Blynk.run();
  timer.run();  
}
