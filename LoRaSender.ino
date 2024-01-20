#define COMPILE_TX
#define freq_US 915E6 // In Canada 915MHz is used
#define SS 8 // default = 10. NSS on LoRa
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
#ifdef COMPILE_TX
void loop() {
  Serial.print("Counter = ");
  Serial.println(counter);
  LoRa.beginPacket();
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
  delay(2000);
}
#endif
