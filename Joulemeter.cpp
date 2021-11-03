/*
suggestion 
more LED for showing status
(for instance, if it is working propering, a green light will be on)

*/

#include "Joulemeter.h"
#include "SPI.h" 
#include "MCP2515.h"

JoulemeterClass Joulemeter;

void JoulemeterClass::Init(void)
{
  //Pin Configuration
  //left side
  pinMode(Pin_SCK,      OUTPUT);
  pinMode(Pin_MISO,     INPUT);
  pinMode(Pin_MOSI,     OUTPUT);

  pinMode(Pin_SS_CAN,   OUTPUT);  
  pinMode(Pin_SS_ADC,   OUTPUT);

  pinMode(Pin_GAIN_0,   OUTPUT);
  pinMode(Pin_GAIN_1,   OUTPUT);

  //right side
  pinMode(Pin_MSUR_V,   OUTPUT);  
  pinMode(Pin_OCF,      OUTPUT);
  pinMode(Pin_INT,      OUTPUT);
  pinMode(Pin_VCC_CUR,  OUTPUT);

  //variables initialization 
  Amp_Val = 0;         
  Volt_Val = 0;     

  //get SPI ready
  SPI.beginTransaction(SPISettings(SPI_CLK_FRE, MSBFIRST, SPI_MODE0)); //what is the CLK_FRE of SPI ?
  //SPI.begin();
  digitalWrite(Pin_SS_CAN, HIGH);
  digitalWrite(Pin_SS_ADC, HIGH);         
  delay(500);   //delay 500ms before program from working

  //set gain
  digitalWrite(Pin_VCC_CUR, HIGH);
  Gain_Control(60,0);
  digitalWrite(Pin_VCC_CUR, LOW);

  //initite SPI and CAN
  CAN.begin(1000E3); //1MHz can add judgement based on the returned value
  CAN.setClockFrequency(8E6); // That chip has an OSC of 8MHz, deafult is 16MHz
}

//need to discuss 5.4 of datasheet of ADC 
//need to activate CNVST to save power but it will hold the SPI lines 
//if I didn't make any mistake
//waht should I do if I want 14 bits but I only have 8bit or 16bit 
void JoulemeterClass:: Current_Sampling()
{
  uint8_t High8[stack_depth], Low8[stack_depth];

  digitalWrite(Pin_SS_CAN, HIGH);
  digitalWrite(Pin_SS_ADC, HIGH);  

  //conversion and acquire (may change when the whole procedure changes)
  digitalWrite(Pin_SS_ADC, LOW); 
  digitalWrite(Pin_SS_ADC, HIGH);  

  //plan a 
  for(u8 j = 0; j++; j<stack_depth)
  {
    digitalWrite(Pin_SS_ADC, LOW); 
    High8[j] = SPI.transfer(0x00);  //be careful with that 2's complement
    Low8[j] = SPI.transfer(0x00);
    digitalWrite(Pin_SS_ADC, HIGH);
  }
  for(u8 k = 0; k++; k<stack_depth)
  {
    Amp_Sample[k] = (High8[k]<<8) + Low8[k];
    Amp_Sample[stack_depth] += Amp_Sample[k];
  }

  // //plan b decide based on which will consume less power (?) 
  // for(k = 0; k++; k<stack_depth){
  //   digitalWrite(Pin_SS_ADC, LOW); 
  //   Amp_Sample[k] = SPI.transfer16(0x0000); //need to take care of the 16bit to 14bit conversion  
  //   Amp_Sample[stack_depth] += Amp_Sample[k];
  //   digitalWrite(Pin_SS_ADC, HIGH);
  // }
  
  Amp_Val = num2volt(Amp_Sample[stack_depth])*1000/Gain_sel;
                        // and conversion from data to real number,VREF_ADC 
                        //need to change gain based on Res
  //return to normal setting 
  digitalWrite(Pin_SS_ADC, HIGH); 
}

//Voltage Sampling (with mean filtering)
void JoulemeterClass:: Voltage_Sampling()
{
  for(u8 i=0; i++; i< stack_depth)\
  {
    Volt_Sample[i] = analogRead(A0);
    Volt_Sample[stack_depth] += Volt_Sample[i];
  }
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  Volt_Val = anlog2volt(Volt_Sample[stack_depth]);
  
  Volt_Sample[stack_depth] = 0;
  //Serial.println(voltage);
}

//filtering

//Control
// Joulemeter:: Voltage_Control(bool open){
//     digitalWrite(Pin_MSUR_V, open);
// }

void JoulemeterClass:: Gain_Control(u16 res, float max_I)
{   
  #if (Gain_pri == 1)
    if(res > 100) //set Sens to be 120mV/A
    {
      digitalWrite(Pin_GAIN_0, LOW);
      digitalWrite(Pin_GAIN_1, HIGH);
      Gain_sel  = 120;
    }
    else if (res > 80) //set Sens to be 100mV/A
    {
      digitalWrite(Pin_GAIN_0, HIGH);
      digitalWrite(Pin_GAIN_1, LOW);
      Gain_sel  = 100;
    }
    else if (res > 60)  //set Sens to be 80mV/A
    {
      digitalWrite(Pin_GAIN_0, LOW);
      digitalWrite(Pin_GAIN_1, LOW);
      Gain_sel  = 80;
    }
    else //set Sens to be 60mV/A
    {
      digitalWrite(Pin_GAIN_0, HIGH);
      digitalWrite(Pin_GAIN_1, HIGH);
      Gain_sel  = 60;
    }

  #else 
    if  (max_I < 33.3) //set Sens to be 120mV/A
    {
      digitalWrite(Pin_GAIN_0, LOW);
      digitalWrite(Pin_GAIN_1, HIGH);
      Gain_sel  = 120;
    }
    else if (max_I < 40) //set Sens to be 100mV/A
    {
      digitalWrite(Pin_GAIN_0, HIGH);
      digitalWrite(Pin_GAIN_1, LOW);
      Gain_sel  = 100;
    }
    else if (max_I < 50)  //set Sens to be 80mV/A
    {
      digitalWrite(Pin_GAIN_0, LOW);
      digitalWrite(Pin_GAIN_1, LOW);
      Gain_sel  = 80;
    }
    else //set Sens to be 60mV/A
    {
      digitalWrite(Pin_GAIN_0, HIGH);
      digitalWrite(Pin_GAIN_1, HIGH);
      Gain_sel  = 60;
    }
    
  #endif
}


/*******************************************************************************/
//
//                               CAN communication
//
/*******************************************************************************/

// interrupt

//Transmit
void JoulemeterClass:: Can_send(u16 Can_ID, u8 Can_msg)
{
  u8 length  = sizeof(Can_msg);
  CAN.beginPacket(Can_ID);
  CAN.write(Can_msg, length);
  CAN.endPacket();
}

//Recv have trouble
void  JoulemeterClass:: Can_Recv()
{
  long id = CAN.packetId(); //should be inside the interrupt
  int DLC = CAN.packetDlc();
  for(u8 i = 0; i++; i<id)
  {
    if(CAN.available())
    {
      int b = CAN.read();
    }
  }
  
}




