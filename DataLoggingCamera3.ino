// CAMERA
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include "memorysaver.h"
//This demo can only work on OV5640_MINI_5MP_PLUS or OV5642_MINI_5MP_PLUS platform.
#define SS 8
#define RST 9
#define DIO0 2
#if !(defined (OV2640_MINI_2MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x00
// set pin 7 as the slave select for the digital pot:
const int CS = 7;
#define SD_CS 10
bool is_header = false;
int total_time = 0;
#if defined (OV2640_MINI_2MP_PLUS)
ArduCAM myCAM( OV2640, CS );
#endif
uint8_t read_fifo_burst(ArduCAM myCAM);
// CAMERA END

//BMP and RTC
#include <RTClib.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp1(4); // hardware SPI
RTC_PCF8523 rtc;

File myFile;
//BMP and RTC END

void setup() {
  uint8_t vid, pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  Serial.println("LoRa Sender");
  // set the CS as an output:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  pinMode(3, OUTPUT);
  // initialize SPI:
  SPI.begin();
  //Reset the CPLD
myCAM.write_reg(0x07, 0x80);
delay(100);
myCAM.write_reg(0x07, 0x00);
delay(100);
  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI interface Error!"));
      delay(1000); continue;
    } else {
      Serial.println(F("SPI interface OK.")); break;
    }
  }
#if defined (OV2640_MINI_2MP_PLUS)
  while (1) {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
      Serial.println(F("ACK CMD Can't find OV2640 module!"));
      delay(1000); continue;
    }
    else {
      Serial.println(F("ACK CMD OV2640 detected.")); break;
    }
  }
#endif
  //Initialize SD Card
  while (!SD.begin(SD_CS))
  {
    Serial.println(F("SD Card Error!")); delay(1000);
  }
  Serial.println(F("SD Card detected."));
    
  //Change to JPEG capture mode and initialize the OV5640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  
  // BMP and RTC
  if (!bmp1.begin()) {
    Serial.println("Sensor BMP280 device 1 was not found.");
    while (1);
  }
  Serial.println("Initialize BMP280 1 completed.");
  delay(2000);  
  
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
  Serial.println("RTC is running.");  
  // BMP and RTC END

  // LoRa setup
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Setup Successful.");
}

void loop() {
  // put your main code here, to run repeatedly:
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
#if defined (OV2640_MINI_2MP_PLUS)
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
#endif
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start capture."));
  total_time = millis();
  while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  Serial.println(F("CAM Capture Done."));
  total_time = millis() - total_time;
  Serial.print(F("capture total_time used (in miliseconds):"));
  Serial.println(total_time, DEC);
  total_time = millis();
  read_fifo_burst(myCAM);
  total_time = millis() - total_time;
  Serial.print(F("save capture total_time used (in miliseconds):"));
  Serial.println(total_time, DEC);
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  // BMP and RTC
  DateTime now = rtc.now();
  
  float pressure = bmp1.readPressure()/100;
  float temperature = bmp1.readTemperature();

  // If the code below is not commented out, the file fails to open for picture saving.
  // The same occurs if you try to save temperature pressure data to the SD.

  //Serial.println("Temperature: ");
  LoRa.beginPacket();   
  LoRa.println(temperature);
  //Serial.println("Pressure: ");       
  LoRa.println(pressure);
  LoRa.print(now.hour(), DEC);
  LoRa.print(':');
  LoRa.print(now.minute(), DEC);
  LoRa.print(':');
  LoRa.println(now.second(), DEC); 
  LoRa.endPacket();   
  Serial.print("package sent");
  // BMP and RTC END
  digitalWrite(3, HIGH);
  delay(1000);
  digitalWrite(3, LOW);
}
uint8_t read_fifo_burst(ArduCAM myCAM)
{
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  static int i = 0;
  static int k = 0;
  char str[16];
  File outFile;
  byte buf[256];
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println("Over size.");
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  i = 0;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);
      //Close the file
      outFile.close();
      Serial.println(F("OK"));
      is_header = false;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      i = 0;
    }
    if (is_header == true)
    {
      //Write image data to buffer if not full
      if (i < 256)
        buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      myCAM.CS_HIGH();
      //Create a avi file
      k = k + 1;
      itoa(k, str, 10);
      strcat(str, ".jpg");
      //Open the new file
      outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
      if (! outFile)
      {
        Serial.println(F("File open failed"));
        while (1);
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }
  myCAM.CS_HIGH();
  return 1;
}
