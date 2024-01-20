#define COMPILE_TX

#define freq_US 915E6
#define SS 10
#define RST 9
#define DIO0 2
#include <LoRa.h>
#include <SPI.h>

int counter = 0;

void setup(){
  Serial.begin(9600);
  while (!Serial);
    LoRa.setPins(SS, RST, DIO0);
  while (!LoRa.begin(915E6)){
    Serial.println(".");
    delay(50);
  }
  Serial.println("LoRa Setup Successful");
}

void loop(){
  Serial.print("Counter = ");
  Serial.println(counter);
  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
  delay(2000);
}
