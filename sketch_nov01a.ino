#include "SPI.h"
#include "MCP2515.h"
#include "Joulemeter.h"


//Can and ADC could have different SPI frequency but I think one is OK 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.available();
  
  CAN.beginPacket(id, dlc, rtr);
  CAN.write(buffer, length);
  CAN.endPacket();
  // the loop routine runs over and over again forever:
  // read the input on analog pin 0:
  CAN.sleep();
  CAN.wakeup();

}
