#include "SPI.h"
#include "MCP2515.h"
#include "Joulemeter.h"


//Can and ADC could have different SPI frequency but I think one is OK 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Joulemeter.Init();
  attachInterrupt(digitalPinToInterrupt(Pin_INT), CAN_Recv_INT, FALLING);
}

void loop() {
  
  //get command to check Voltage and circuit;
  //if(command_flag == FLG_Sample)
  {
    Joulemeter.Current_Sampling();
    Joulemeter.Voltage_Sampling();

    Joulemeter.Can_send(0x01, {0});
  }

  delay(1000); //delay 1s when nothing happens
}
 
//interrupt callback function
void CAN_Recv_INT()
{
  Joulemeter.Can_Recv();
}