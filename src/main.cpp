#include <Arduino.h>

const int MOTOR_PIN     = 7;
const int LED_PIN_CHAUF = 2;
const int LED_PIN_CLIM  = 4;

bool LedStatusChauf = false;
bool LedStatusClim = false;

int Intensite = 0;


#include <Wire.h>
#include <Adafruit_AHTX0.h>  
#include <stdio.h> 
#include "WIFIConnector_MKR1000.h"
#include "MQTTConnector.h"
#include <SPI.h>
#include "RTClib.h"
#include <SD.h>
#include <Servo.h>


using namespace std;


//declaration des constantes
const int delayFile = 5000;
const int delayTB = 30000;

Adafruit_AHTX0 aht;
sensors_event_t humidity, temp;
RTC_DS3231 rtc;
Servo myservo; // creation de l'objet servo

char buf [1000] = {0};
char dataString [100] = {0};
String chain1;

//declaration des variables
const int chipSelect = 4;
int n, nbLines = 0, fileLength = 0;
int pos = 0;



void setup () {
  
  Serial.begin(9600);

  
  myservo.attach(1);

  pinMode(LED_PIN_CHAUF, OUTPUT);
  pinMode(LED_PIN_CLIM, OUTPUT);

  if (aht.begin()) {
    Serial.println("Found AHT20");
  } else {
    Serial.println("Didn't find AHT20");
  }


  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  wifiConnect();        
  MQTTConnect(); 
}


boolean runEveryShort(unsigned long interval){

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){

    previousMillis = currentMillis;
    return true;
  }
  return false;

}

boolean runEveryShortTb(unsigned long interval){

  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){

    previousMillis = currentMillis;
    return true;
  }
  return false;

}


void loop () {
  
  ClientMQTT.loop(); 

  if (runEveryShort(delayFile)){

    aht.getEvent(&humidity, &temp);   //checking AHT20 existence
    DateTime now = rtc.now();         //initializing now to current time
    
    n = sprintf(dataString, "{\"ts\":%f,\"values\":{\"TMP\":%f,\"HUM\":%f}}" , (double)(now.unixtime() + 13440) * 1000, temp.temperature, humidity.relative_humidity);
    
    //open file or create and open if it doesent exist
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    //create string in file
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println(dataString);
      nbLines += 1;
      
    }else {
      Serial.println("error opening datalog.txt");
    }
  }


  if (runEveryShortTb(delayTB)){

    /*
    status = WL_IDLE_STATUS;
    wifiConnect();        
    MQTTConnect();
    */

    File dataFile1 = SD.open("datalog.txt", FILE_READ);
    while (dataFile1.available())
    {
      chain1 = dataFile1.readStringUntil('\n');
      Serial.println(chain1);
      sendPayloadString(chain1); 
    }

    
  if (LedStatusClim)
    {
      analogWrite(LED_PIN_CLIM ,255);
      myservo.write(179);
    }
    else
    {
      analogWrite(LED_PIN_CLIM,0);
      myservo.write(90);
    }


  if (LedStatusChauf)
    {
      analogWrite(LED_PIN_CHAUF, 255);
      myservo.write(0);
    }else{
      myservo.write(90);
      analogWrite(LED_PIN_CHAUF,0);
    }

    SD.remove("datalog.txt");
    Serial.println("file deleted");

    //WiFi.disconnect();
    //WiFi.end();
  }
}









