#include <SD.h> //sd library
#include <SPI.h> //Serial Peripheral Interface library
#include <RTClib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp1(4); // hardware SPI
RTC_PCF8523 rtc;

File myFile;

void setup() {
  pinMode(10, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Starting BMP280 device 1...");

  // BMP Initialization
  if (!bmp1.begin()) {
    Serial.println("Sensor BMP280 device 1 was not found.");
    while (1);
  }
  Serial.println("Initialize BMP280 1 completed.");
  delay(2000);

  // RTC Initialization

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(3);
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    delay(2500);
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start();

  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
    if (SD.exists("dataLog.csv")) //check to see if dataLog.csv exists
    {
      Serial.println("dataLog.csv already exists");
    }
    else
    {
      //If dataLog.csv does not exist, create it and write the column headings
      Serial.println("dataLog.csv created during setup");
      myFile = SD.open("dataLog.csv", FILE_WRITE);
 
      myFile.print("TEMPERATURE");
      myFile.print(",");     
      myFile.print("PRESSURE");
      myFile.print(",");     
      myFile.println("TIME");
      
      myFile.close();
    }
      
      //SET UP COLUMN HEADINGS IN SERIAL MONITOR
      Serial.print("TEMPERATURE");
      Serial.print("\t");     
      Serial.print("PRESSURE");
      Serial.print("\t");     
      Serial.println("TIME");      

  } 
  
  else
  {
    Serial.println("SD card initialization failed");
    return;
  }

}

void loop() {

  DateTime now = rtc.now();
  
  float pressure = bmp1.readPressure()/100;
  float temperature = bmp1.readTemperature();

  Serial.print(temperature);
  Serial.print("\t\t");     
  Serial.print(pressure);
  Serial.print("\t\t");
   
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);  

  //PRINT TO FILE
  myFile = SD.open("dataLog.csv", FILE_WRITE);
  if (myFile) {    
    myFile.print(temperature);
    myFile.print(",");     
    myFile.print(pressure);

    myFile.print(",");
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(' ');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.println(now.second(), DEC);

    myFile.close(); // close the file
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file");
  }

  delay(1000);
}
