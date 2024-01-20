#define COMPILE_RX

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
  int packetSize = LoRa.parsePacket();
  if (packetSize){
    Serial.print("Counter = ");
    while (LoRa.available()){
      String data = LoRa.readString();
      Serial.print(data);
    }
    Serial.print(" , with RRSI = ");
    Serial.println(LoRa.packetRssi());
  }
}
