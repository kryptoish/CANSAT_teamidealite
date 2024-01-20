#define COMPILE_RX
#define freq_US 915E6 // In Canada 915MHz is used
#define SS 10 // default = 10. NSS on LoRa
#define RST 9 // default = 9, -1 = disable (connect RST pin of MCU with RST of LoRa)
#define DIO0 2 // default = 2. Most be interrupt capable.
#include <LoRa.h>
#include <SPI.h>
int counter = 0;
void setup(){
Serial.begin(115200);
while (!Serial);
LoRa.setPins(SS, RST, DIO0);
while (!LoRa.begin(915E6)){
Serial.println(".");
delay(50);
}
Serial.println("LoRa Setup Successful.");
}

#ifdef COMPILE_RX
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
#endif
