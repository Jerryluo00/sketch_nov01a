#ifndef _JOULEMETER_H_
#define _JOULEMETER_H_

#include <Arduino.h>
//
//all kinds of data and data type need to be adjusted
//

/*******************************************************************************/
//
//                              Gobal definitions
//
/*******************************************************************************/

//left side
#define Pin_SCK     13
#define Pin_MISO    12
#define Pin_MOSI    11
#define Pin_SS_CAN  10
#define Pin_SS_ADC  9
#define Pin_GAIN_0  8
#define Pin_GAIN_1  7

//right side
#define Pin_MSUR_V  4
#define Pin_OCF     3
#define Pin_INT     2
#define Pin_VCC_CUR 1

#define Pin_A_H     A3
#define Pin_A_L     A2
#define Pin_Thermal A1
#define Pin_BatVolt A0

#define SPI_CLK_FRE 15000000 //15MHz 


/*******************************************************************************/
//
//                         Joulemeter definitions
//
/*******************************************************************************/

#define stack_depth 8     //set it for now
#define CAN_ID_I    0x01  //Can ID for current output
#define CAN_ID_V    0x02  //Can ID for Voltage output

#define VREF_ADC  5   //not sure

#define num2volt(x)           (x/stack_depth/(8191/ VREF_ADC )) // 8192 = 2^13
#define anlog2volt(x)         ((5.0 / 1023.0/ stack_depth)*x)
#define MSUR_Volt_Ctrl(open)   digitalWrite(Pin_MSUR_V, open)

#define Gain_pri  1   //1: resolution first 2: max current first 

class JoulemeterClass{
  public:
    // Constructor 
    //Joulemeter();

    //Initialization
    void Init();

    //Sampling
    void Current_Sampling();
    void Voltage_Sampling();

    //Filtering 
    //will add it later

    //Control
    //void MSUR_Voltage_Control(bool open);
    void Gain_Control(u16 res, float max_I);

    //Transmit
    void Can_send(u16 Can_ID, u8* Can_msg);
    void Can_Recv();

    //private: //not really sure if it works
    //data
    u16 Amp_Sample[stack_depth+1] = {0};  //current data of several times of sampling, last position is used to hold the sum
    float Amp_Val = 0;                  //current data after filtering
    int Volt_Sample[stack_depth+1] = {0}; //Voltage data of several times of sampling
    float Volt_Val = 0;                 //Voltage data after filtering

    uint8_t Gain_sel = 60;               //record the current setting of gain_sel, [0] refer to sel_0, [1] refer to sel_1
    
    uint8_t command_flag;
    
};


#endif // _JOULEMETER_H_

extern JoulemeterClass Joulemeter;