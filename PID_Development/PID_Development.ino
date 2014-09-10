/*
PID development with serving data through Ethernet.

Started by Nate and Jianshi.
Finished by Craig (6/14).
*/

#include <SPI.h>
#include <AD7327.h>
#include <MCP23S17.h>
#include <SpiRAM.h>
#include <Ethernet.h>

int freeRam () {
  //This only takes up space in compilation if it is called.
  //http://playground.arduino.cc/Code/AvailableMemory
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

//Must make object for each chip on board
SpiRAM sram;
AD7327 adc;
MCP dio;

// Program Run Parameters
const static bool USE_SERIAL = false;

// SCR trigger parameters
//const static int16_t _ADC_IN = 0;
//const static int16_t _ADC_IN = 1;
//const static int16_t _ADC_IN = 2;
//const static int16_t _ADC_IN = 3;
//const static int16_t _ADC_IN = 4;
const static int16_t RESERVOIR_ADC_IN   = 5;
const static int16_t NUFEREN_ADC_IN     = 6;
//const static int16_t _ADC_IN = 7;

//const static int16_t _DIO_IN           = 0;//DIOA0
//const static int16_t _DIO_OUT          = 1;//DIOA1
const static int16_t COLD_CITY_WATER_FLOW_SWITCH_TOTALIZER_DIO_IN = 2;//DIOA2
//const static int16_t _DIO_OUT          = 3;//DIOA3
//const static int16_t _DIO_IN           = 4;//DIOA4
//const static int16_t _DIO_OUT          = 5;//DIOA5
const static int16_t RESERVOIR_LOOP_FLOW_SWITCH_TOTALIZER_DIO_IN = 6;//DIOA6
const static int16_t RESERVOIR_DIO_OUT          = 7;//DIOA7
//const static int16_t _DIO_OUT          = 8;//DIOB0
const static int16_t LINE_TRIGGER_DIO_IN        = 9;//DIOB1
//const static int16_t _DIO_OUT       = 10;//DIOB2 Chip output not working?
const static int16_t NUFERN_INTERLOCK_DIO_IN        = 11;//DIOB3
//const static int16_t _DIO_OUT       = 12;//DIOB4
//const static int16_t _DIO_IN        = 13;//DIOB5
const static int16_t NUFEREN_DIO_OUT  = 14;//DIOB6  // Pin 14 is connected to Triac #1.
//const static int16_t _DIO_IN        = 15;//DIOB7

//The arrays are indexed interms of "..._ADC_IN"

//Parameters to Change
const static int16_t NUFEREN_VSET = 2710; //24C = 10.7KOhm = 2970 err voltage. (2.5mK/digit) (colder is higher)
const static int16_t RESERVOIR_VSET = 4100;//3850 -> 21C = 2690KOhm. (3mK/digit) 3400 err voltage for ~22.5C (colder is higher err voltage)
//End change

int16_t dioOutChannel[8] = {1,3,5,14,10,RESERVOIR_DIO_OUT,NUFEREN_DIO_OUT,12};//These are the pins on the dio chip aka. DIOA0,DIOA1,...DIOB7 - they are hard wired to the scrs
bool enable[8] = {HIGH_MCP,HIGH_MCP,HIGH_MCP,HIGH_MCP,HIGH_MCP,HIGH_MCP,HIGH_MCP,HIGH_MCP}; // enables output channels when LOW_MCP
//const int T60 = 1523*2; // short loop cycles per 60Hz period - tweak after measuring cycle time. Calculated by sampling digital Read.
//const static int T60 = 2470; // short loop cycles per 60Hz period - needs tweaking . Calculated by looking at a delay in the first for loop.
const static int16_t maxDelay = 680; //680;//730;// REVISIT THIS. ODD CHANGING OF DELAY PROBLEM. There's about 45us long until zero crossing falling edge. NB: About 9.5 microseconds per delay time int.
//const static int padding= 350;  // minimum deadtime for scr trigger before end of 60Hz half-cycle
int16_t Rslope[8] = {1,1,1,1,1,1,1,1};
const static float errVoltageTomKConversion[8] = {0,0,0,0, 0,3.5,2.5,0};//conversion from the error voltage to milli Kelvin.
const static char description[8][5] = {"None","None","None","None", "None","Res0","Nuf0","None"};//description of the inputs
bool interlock_condition = false;

// RAM chip control - We partition the chip into two halves. One for long term memory - once a second, other short term, 60Hz.
const static uint32_t numBytesInSRAM = 0x1FFFF;// = 131071 bytes. 1Mbit - 128K x 8 bit organization
const static uint32_t numBytesInSRAM0 = numBytesInSRAM - 1 / 2;// = 65535 = 2^16 - 1bytes. SRAM half that records data at 60Hz = 68sec
const static uint32_t numBytesInSRAM1 = numBytesInSRAM - 1 / 2;//SRAM half that records data at 1/loopsPerSRAM1Sample Hz.
const static uint16_t loopsPerSRAM1Sample = 4*60; //= every 4 sec => SRAM memory enough for 4.5hrs.
const static int32_t ponterSRAM0Start = 0x00000000;
int32_t pointerSRAM0 = ponterSRAM0Start; // pointer to current memory address for writing samples 60Hz.
int32_t pointerSRAM0Delay = ponterSRAM0Start;// pointer to time delayed memory address for reading past data that was taken at 60Hz.
const static int32_t ponterSRAM1Start = numBytesInSRAM0 + 1;
int32_t pointerSRAM1 = ponterSRAM1Start; // pointer to current memory address for writing samples 1/loopsPerSRAM1Sample Hz.
int32_t pointerSRAM1Delay = ponterSRAM1Start;// pointer to time delayed memory address for reading past data that was taken at 1/loopsPerSRAM1Sample Hz.
uint8_t ramBuffer[16] = {0};  // 16 byte buffer. Each ADC sample is a consectutive 16bits. Used to read/write blocks to SRAM chip
uint16_t loopCounter = 0;

// dynamic variables for PIDs
//adc channels
const static uint8_t maskChannelOn = 0b11100000;
const static uint8_t maskChannelOff = 0b00011111;
const static uint16_t maskChannelOff16 = 0b0001111111111111;

int16_t Vadc[8] = {0}; // holds ADC read values
int16_t VadcSRAM0[8] = {0}; // holds time-delayed ADC read values 60Hz
int16_t VadcSRAM1[8] = {0}; // holds time-delayed ADC read values 1/loopsPerSRAM1Sample Hz
int16_t Vset[8] = {4000,4000,4000, 4000,4000,4000,4000,4000}; // holds set points for error calculation
static const int16_t initialStart = maxDelay;
int16_t outphase[8] = {0};//initialStart,initialStart,initialStart,initialStart,initialStart,initialStart,initialStart,initialStart}; // holds short-loop cycle counts after line trigger on which to trigger SCRs

// Enter a MAC address and IP address for your Arduino.
uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x0F, 0x03, 0x3D};//This is from the sticker on the physical Arduino.
IPAddress ip(10,1,1,200);

// Initialize the Ethernet server library with the IP address and port you want to use 
EthernetServer server(80);// (port 80 is default for HTTP):
bool currentLineIsBlank = true;
EthernetClient client;
static const int16_t TCPBufferLength = 642+42;
//uint8_t TCPBuffer[TCPBufferLength] = {0};
bool isSendingData = false;
int16_t TCPIndex = 0;
const static bool readingTCP = false;

//FlowSwitches
const int16_t totalNumFlowAverages = 120;
const int8_t pulsesPerGallon_CCW = 20;
const int8_t pulsesPerGallon_RL = 20;
int16_t loopsOpen_CCW = 0;
int16_t loopsOpen_RL = 0;
int16_t loopsClosed_CCW = 0;
int16_t loopsClosed_RL = 0;
bool switchOpen_CCW = false;
bool switchOpen_RL = false;
float pulsesPerSecond_CCW = 0;
float pulsesPerSecond_RL = 0;
int16_t numFlowAverages_CCW = 0;
int16_t numFlowAverages_RL = 0;
float sumPulsesPerSecond_CCW = 0;
float sumPulsesPerSecond_RL = 0;
float avePulsesPerSecond_CCW = 0;
float avePulsesPerSecond_RL = 0;

//Parameters to change
//PID parameters for gains - these should be 8-parameter arrays
float Ku[8]={1,1,1,1,1,100,100,1}; // setting for K at which loop oscillates with proportional feedback. The thermal reservoir has an integrating property to it
float Tu[8]={840,840,840,840,840,60*290*5,60*120,840}; // period of oscillation with proportional feedback
//End Change

int16_t N[8]={12,12,12,12,12,12,12,12}; // derivative is lowpass filtered at a time-constant of Td/N - good choice 8-20
int16_t hdelay[8]={100,100,100,100,100,100,100,100}; //delay in 60Hz cycle count between values used for derivative and integral calculation
int16_t hdelaySRAM1[8]={100,100,100,100,100,100,100,100}; //delay in 1/loopsPerSRAM1Sample Hz cycle count. For derivative and integral calc.
float b[8]={0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5}; // Setpoint weighting to avoid impulse bumps when resetting setpoint 
int16_t UPSAT[8]={8000,8000,8000,8000,8000,8000,8000,8000};//4000,4000,4000,4000,4000,4000,4000,4000}; // Upper saturation value for PID output
int16_t LOWSAT[8]={10,10,10,10,10,10,10,10}; // Lower saturation value for PID output

float K[8] = {0}; // overall scaling of gain
float Ti[8] = {0}; // "integral time" - inverse to integral gain 
float Td[8] = {0}; // effective extrapolation time of derivative gain - larger is larger gain
float Tt[8] = {0}; // anti-windup time constant - good choice is (Ti*Td)^.5

float bi[8] = {0};  // integral gain
float ad[8] = {0}; //derivative coefficient
float bd[8] = {0}; // derivative gain
float a0[8] = {0}; //integral coefficient

float P[8] = {0};  // proportional signal
float D[8] = {0}; // Differential gain with filtering
float v[8] = {0}; // idealized PID response
float u[8] = {0}; // saturated actuator simulation
float I[8] = {0};  // Integrator signal with wind-up prevention

const static int16_t microsecondDelayfor220Phase = 1200;

/**/

void setup(){
if (USE_SERIAL){Serial.begin(9600);Serial.println("begin setup");}
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  if (USE_SERIAL){Serial.print("server is at ");Serial.println(Ethernet.localIP());}

  //start the SPI library:
  SPI.begin();

  // initialize dio channels as in/out
  dio.pinMode_MCP(0b1111111111111111);  // single argument to the function as 0x(portB)(portA) 1 = input 0 = output. Inputs are high impedance states
  for(int i = 0; i < 8; i++)  {dio.pinMode_MCP(dioOutChannel[i], OUTPUT_MCP);}
 
    
//Initialize dio pin mapping
//dioOutChannel[_ADC_IN] = 1;                    //Pin 1, GPA1, DIOA1, CLK4, SCR#5
//dioOutChannel[_ADC_IN] = 3;                    //Pin 3, GPA3, DIOA3, CLK5, SCR#6
//dioOutChannel[_ADC_IN] = 5;                    //Pin 5, GPA5, DIOA5, CLK6, SCR#7
//dioOutChannel[_ADC_IN] = 8;                    //Pin 8, GPB0, DIOB0, CLK3, SCR#4
//dioOutChannel[_ADC_IN] = 10;                   //Pin 10, GPB2, DIOB2, CLK2, SCR#3
dioOutChannel[RESERVOIR_ADC_IN] = RESERVOIR_DIO_OUT;    //Pin 7, GPA7, DIOA7, Green wire to PWM 2-10 V.
dioOutChannel[NUFEREN_ADC_IN] = NUFEREN_DIO_OUT;        //Pin 14, GPB6, DIOB6, CLK0, SCR#1.
//dioOutChannel[_ADC_IN] = 12;                   //Pin 12, GPB4, DIOB4, CLK1, SCR#2
  
  dio.pinForFastRead = (1 << (LINE_TRIGGER_DIO_IN - 8));
  
  Vset[RESERVOIR_ADC_IN] = RESERVOIR_VSET;
  dioOutChannel[RESERVOIR_ADC_IN] = RESERVOIR_DIO_OUT;
  enable[RESERVOIR_ADC_IN] = LOW_MCP;
  Rslope[RESERVOIR_ADC_IN] = -1;//R coeff on thermistor goes to small R when T is high. When Reservoir is hotter than Tset (Rtherm < Rset), increase phase.

  Vset[NUFEREN_ADC_IN] = NUFEREN_VSET;
  dioOutChannel[NUFEREN_ADC_IN] = NUFEREN_DIO_OUT;
  enable[NUFEREN_ADC_IN] = LOW_MCP;
  Rslope[NUFEREN_ADC_IN] = -1;//R coeff on thermistor goes to small R when T is high.

  //Initialize SRAM
  //fill SRAM with hex FF's so unwritten bytes are easier to identify in ram dumps 
  sram.fillBytes(0,0x00,numBytesInSRAM);
  //check SRAM chip has enough memory
  //...
  
  //Pull up Ethernet CS.
  pinMode(10,OUTPUT);//Ethernet CS
  digitalWrite(10,HIGH);

  // calculate control parameters
  for (int i=0; i<8; i++)
  {
    K[i]=0.6*Ku[i]; // overall scaling of gain. 
    Ti[i]=0.5*Tu[i]*300; // "integral time" - inverse to integral gain . CP added the *300
    Td[i]=0.125*Tu[i]; // "derivative time" - effective extrapolation time of derivative gain 
    Tt[i]=0.25*Tu[i]; // anti-windup time constant - good choice is (Ti*Td)^.5=0.25*Tu
    
    bi[i]=K[i]*hdelay[i]/Ti[i];  // integral gain
    ad[i]=(2*Td[i]-N[i]*hdelay[i])/(2*Td[i]+N[i]*hdelay[i]); //derivative coefficient
    bd[i]=2*K[i]*N[i]*Td[i]/(2*Td[i]+N[i]*hdelay[i]); // derivative gain
    a0[i]=hdelay[i]/Tt[i]; //integral coefficient
  }

  /**/
  if (USE_SERIAL) {Serial.println("setup completed\n");}
}


void writeToClient(EthernetClient client){
 //Do not change the order of any of the contributions to the TCPBuffer because
 //The program saving this to braid is counting on this order to TCPBuffer.
 
      //Serial.println("df4");
  // Write Setup information to TCPBuffer
  uint8_t const * p;
  
  //arduino mac address
  client.write(mac,sizeof(mac));
  
  //maxDelay
  for(int bytes = 0; bytes < sizeof(maxDelay); bytes++)
  {
    client.write((uint8_t) (maxDelay >> ((sizeof(maxDelay)-1)*8 - 8*bytes)));
  }

  //microsecondDelayfor220Phase
  for(int bytes = 0; bytes < sizeof(microsecondDelayfor220Phase); bytes++)
  {
    client.write((uint8_t) (microsecondDelayfor220Phase >> ((sizeof(microsecondDelayfor220Phase)-1)*8 - 8*bytes)));
  }

  /*
  Serial.println(totalNumFlowAverages);
  Serial.println(loopsOpen_CCW);
  Serial.println(loopsClosed_CCW);
  Serial.println(switchOpen_CCW);
  Serial.println(pulsesPerSecond_CCW);
  Serial.println(numFlowAverages_CCW);
  Serial.println(avePlusesPerSecond_CCW/numFlowAverages_CCW);
  Serial.println(loopsOpen_RL);
  Serial.println(loopsClosed_RL);
  Serial.println(switchOpen_RL);
  Serial.println(pulsesPerSecond_RL);
  Serial.println(numFlowAverages_RL);
  Serial.println(avePlusesPerSecond_RL/numFlowAverages_RL);
  Serial.println();
  

const int16_t totalNumFlowAverages = 10;
int16_t loopsOpen_CCW = 0;
int16_t loopsOpen_RL = 0;
int16_t loopsClosed_CCW = 0;
int16_t loopsClosed_RL = 0;
bool switchOpen_CCW = false;
bool switchOpen_RL = false;
float pulsesPerSecond_CCW = 0;
float pulsesPerSecond_RL = 0;
int16_t numFlowAverages_CCW = 0;
int16_t numFlowAverages_RL = 0;
float avePlusesPerSecond_CCW = 0;
float avePlusesPerSecond_RL = 0;
*/

  //Begin Flow
  //totalNumFlowAverages
  for(int bytes = 0; bytes < sizeof(totalNumFlowAverages); bytes++)
  {
    client.write((uint8_t) (totalNumFlowAverages >> ((sizeof(totalNumFlowAverages)-1)*8 - 8*bytes)));
  }
  //pulsesPerGallon_CCW
  for(int bytes = 0; bytes < sizeof(pulsesPerGallon_CCW); bytes++)
  {
    client.write((uint8_t) (pulsesPerGallon_CCW >> ((sizeof(pulsesPerGallon_CCW)-1)*8 - 8*bytes)));
  }
  //loopsOpen_CCW
  for(int bytes = 0; bytes < sizeof(loopsOpen_CCW); bytes++)
  {
    client.write((uint8_t) (loopsOpen_CCW >> ((sizeof(loopsOpen_CCW)-1)*8 - 8*bytes)));
  }
  //loopsClosed_CCW
  for(int bytes = 0; bytes < sizeof(loopsClosed_CCW); bytes++)
  {
    client.write((uint8_t) (loopsClosed_CCW >> ((sizeof(loopsClosed_CCW)-1)*8 - 8*bytes)));
  }
  //switchOpen_CCW
  for(int bytes = 0; bytes < sizeof(switchOpen_CCW); bytes++)
  {
    client.write((uint8_t) (switchOpen_CCW >> ((sizeof(switchOpen_CCW)-1)*8 - 8*bytes)));
  }
  //pulsesPerSecond_CCW
  p = reinterpret_cast<unsigned uint8_t const *>(&pulsesPerSecond_CCW);
  for(int bytes = 0; bytes < sizeof(pulsesPerSecond_CCW); bytes++)
  {
    client.write((uint8_t) p[sizeof(pulsesPerSecond_CCW)-1 - bytes]);
  }
  //numFlowAverages_CCW
  for(int bytes = 0; bytes < sizeof(numFlowAverages_CCW); bytes++)
  {
    client.write((uint8_t) (numFlowAverages_CCW >> ((sizeof(numFlowAverages_CCW)-1)*8 - 8*bytes)));
  }
  //sumPulsesPerSecond_CCW
  p = reinterpret_cast<unsigned uint8_t const *>(&sumPulsesPerSecond_CCW);
  for(int bytes = 0; bytes < sizeof(sumPulsesPerSecond_CCW); bytes++)
  {
    client.write((uint8_t) p[sizeof(sumPulsesPerSecond_CCW)-1 - bytes]);
  }
  //avePulsesPerSecond_CCW
  p = reinterpret_cast<unsigned uint8_t const *>(&avePulsesPerSecond_CCW);
  for(int bytes = 0; bytes < sizeof(avePulsesPerSecond_CCW); bytes++)
  {
    client.write((uint8_t) p[sizeof(avePulsesPerSecond_CCW)-1 - bytes]);
  }
  //pulsesPerGallon_RL
  for(int bytes = 0; bytes < sizeof(pulsesPerGallon_RL); bytes++)
  {
    client.write((uint8_t) (pulsesPerGallon_RL >> ((sizeof(pulsesPerGallon_RL)-1)*8 - 8*bytes)));
  }
  //loopsOpen_RL
  for(int bytes = 0; bytes < sizeof(loopsOpen_RL); bytes++)
  {
    client.write((uint8_t) (loopsOpen_RL >> ((sizeof(loopsOpen_RL)-1)*8 - 8*bytes)));
  }
  //loopsClosed_RL
  for(int bytes = 0; bytes < sizeof(loopsClosed_RL); bytes++)
  {
    client.write((uint8_t) (loopsClosed_RL >> ((sizeof(loopsClosed_RL)-1)*8 - 8*bytes)));
  }
  //switchOpen_RL
  for(int bytes = 0; bytes < sizeof(switchOpen_RL); bytes++)
  {
    client.write((uint8_t) (switchOpen_RL >> ((sizeof(switchOpen_RL)-1)*8 - 8*bytes)));
  }
  //pulsesPerSecond_RL
  p = reinterpret_cast<unsigned uint8_t const *>(&pulsesPerSecond_RL);
  for(int bytes = 0; bytes < sizeof(pulsesPerSecond_RL); bytes++)
  {
    client.write((uint8_t) p[sizeof(pulsesPerSecond_RL)-1 - bytes]);
  }
  //numFlowAverages_RL
  for(int bytes = 0; bytes < sizeof(numFlowAverages_RL); bytes++)
  {
    client.write((uint8_t) (numFlowAverages_RL >> ((sizeof(numFlowAverages_RL)-1)*8 - 8*bytes)));
  }
  //sumPulsesPerSecond_RL
  p = reinterpret_cast<unsigned uint8_t const *>(&sumPulsesPerSecond_RL);
  for(int bytes = 0; bytes < sizeof(sumPulsesPerSecond_RL); bytes++)
  {
    client.write((uint8_t) p[sizeof(sumPulsesPerSecond_RL)-1 - bytes]);
  }
  //avePulsesPerSecond_RL
  p = reinterpret_cast<unsigned uint8_t const *>(&avePulsesPerSecond_RL);
  for(int bytes = 0; bytes < sizeof(avePulsesPerSecond_RL); bytes++)
  {
    client.write((uint8_t) p[sizeof(avePulsesPerSecond_RL)-1 - bytes]);
  }
  //End Flow
  
  //Per Channel Information
  for(int channel = 0; channel < 8; channel++)
  {
    
      //Serial.println("f");
    //dioOutChannel
    for(int bytes = 0; bytes < sizeof(dioOutChannel[channel]); bytes++)
    {
      client.write((uint8_t) (dioOutChannel[channel] >> ((sizeof(dioOutChannel[channel])-1)*8 - 8*bytes)));
    }
    //enable
    for(int bytes = 0; bytes < sizeof(enable[channel]); bytes++)
    {
      client.write((uint8_t) (enable[channel] >> ((sizeof(enable[channel])-1)*8 - 8*bytes)));
    }
    
    //Rslope
    for(int bytes = 0; bytes < sizeof(Rslope[channel]); bytes++)
    {
      client.write((uint8_t) (Rslope[channel] >> ((sizeof(Rslope[channel])-1)*8 - 8*bytes)));
    }
    
    //Vset
    for(int bytes = 0; bytes < sizeof(Vset[channel]); bytes++)
    {
      client.write((uint8_t) (Vset[channel] >> ((sizeof(Vset[channel])-1)*8 - 8*bytes)));
    }
    //hdelay
    for(int bytes = 0; bytes < sizeof(hdelay[channel]); bytes++)
    {
      client.write((uint8_t) (hdelay[channel] >> ((sizeof(hdelay[channel])-1)*8 - 8*bytes)));
    }
    
    //N
    for(int bytes = 0; bytes < sizeof(N[channel]); bytes++)
    {
      client.write((uint8_t) (N[channel] >> ((sizeof(N[channel])-1)*8 - 8*bytes)));
    }
    
    //proportional oscillation amplitude
    p = reinterpret_cast<unsigned uint8_t const *>(&Ku[channel]);
    for(int bytes = 0; bytes < sizeof(Ku[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(Ku[channel])-1 - bytes]);
    }
    //proportional oscillation time
    p = reinterpret_cast<unsigned uint8_t const *>(&Tu[channel]);
    for(int bytes = 0; bytes < sizeof(Tu[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(Tu[channel])-1 - bytes]);
    }
    
    //Overall scaling gain
    p = reinterpret_cast<unsigned uint8_t const *>(&K[channel]);
    for(int bytes = 0; bytes < sizeof(K[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(K[channel])-1 - bytes]);
    }
    
    //integral time
    p = reinterpret_cast<unsigned uint8_t const *>(&Ti[channel]);
    for(int bytes = 0; bytes < sizeof(Ti[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(Ti[channel])-1 - bytes]);
    }
    
    //derivative time
    p = reinterpret_cast<unsigned uint8_t const *>(&Td[channel]);
    for(int bytes = 0; bytes < sizeof(Td[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(Td[channel])-1 - bytes]);
    }
    
    //anti-windup constant
    p = reinterpret_cast<unsigned uint8_t const *>(&Tt[channel]);
    for(int bytes = 0; bytes < sizeof(Tt[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(Tt[channel])-1 - bytes]);
    }
        
    //integral coefficient
    p = reinterpret_cast<unsigned uint8_t const *>(&a0[channel]);
    for(int bytes = 0; bytes < sizeof(a0[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(a0[channel])-1 - bytes]);
    }
    
    //integral gain
    p = reinterpret_cast<unsigned uint8_t const *>(&bi[channel]);
    for(int bytes = 0; bytes < sizeof(bi[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(bi[channel])-1 - bytes]);
    }
    
    //derivative coefficient
    p = reinterpret_cast<unsigned uint8_t const *>(&ad[channel]);
    for(int bytes = 0; bytes < sizeof(ad[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(ad[channel])-1 - bytes]);
    }
    
    //derivative gain
    p = reinterpret_cast<unsigned uint8_t const *>(&bd[channel]);
    for(int bytes = 0; bytes < sizeof(bd[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(bd[channel])-1 - bytes]);
    }
    
    //errVoltageTomKConversion
    p = reinterpret_cast<unsigned uint8_t const *>(&errVoltageTomKConversion[channel]);
    for(int bytes = 0; bytes < sizeof(errVoltageTomKConversion[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(errVoltageTomKConversion[channel])-1 - bytes]);
    }
    
    //Description
    for(int bytes = 0; bytes < sizeof(description[channel])-1; bytes++)//-1 to not include the '\0' termination char.
    {
      client.write((uint8_t) description[channel][bytes]);
    }
    
    //Proportional response
    p = reinterpret_cast<unsigned uint8_t const *>(&P[channel]);
    for(int bytes = 0; bytes < sizeof(P[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(P[channel])-1 - bytes]);
    }
    
    //integral response
    p = reinterpret_cast<unsigned uint8_t const *>(&I[channel]);
    for(int bytes = 0; bytes < sizeof(I[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(I[channel])-1 - bytes]);
    }
    
    //derivative response
    p = reinterpret_cast<unsigned uint8_t const *>(&D[channel]);
    for(int bytes = 0; bytes < sizeof(D[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(D[channel])-1 - bytes]);
    }
    
    //combined response saturated
    p = reinterpret_cast<unsigned uint8_t const *>(&u[channel]);
    for(int bytes = 0; bytes < sizeof(u[channel]); bytes++)
    {
      client.write((uint8_t) p[sizeof(u[channel])-1 - bytes]);
    }
    
    //outphase
    for(int bytes = 0; bytes < sizeof(outphase[channel]); bytes++)
    {
      client.write((uint8_t) (outphase[channel] >> ((sizeof(outphase[channel])-1)*8 - 8*bytes)));
    }
    
    //ErrorVoltage
    for(int bytes = 0; bytes < sizeof(Vadc[channel]); bytes++)
    {
      client.write((uint8_t) (Vadc[channel] >> ((sizeof(Vadc[channel])-1)*8 - 8*bytes)));
    }
  }
  //End writing binary data to TCP client.
}

void readInAnalogInputs(){
   adc.analogRead();
   for (int channel = 0; channel < 8; channel++)
   {
    Vadc[channel] = (uint16_t) (adc._data[channel] & maskChannelOff16);//we need to make sure we work with the voltages casted into signed ints
    ramBuffer[2 * channel] = (uint8_t) ((0b0001111111111111 & adc._data[channel]) >> 8);//store high byte.
    ramBuffer[2 * channel + 1] = (uint8_t) adc._data[channel];//then add low byte after.
    //TCPBuffer[2 * channel +1] = (uint8_t) ramBuffer[2 * channel];
    //TCPBuffer[2 * channel ] = (uint8_t) ramBuffer[2 * channel + 1];
   }
}

void saveInputsToRam(int16_t loopCounter){
 //save the results to RAM, look-up time-delayed values to create a sampling interval longer than 60Hz period
  pointerSRAM0 = ((pointerSRAM0+16) % numBytesInSRAM0); // advance and wrap-around pointer
  sram.writeBuffer(pointerSRAM0,ramBuffer,16); // write these samples (from the previous loop sampling) to memory
  for (int channel = 0; channel < 8; channel++)
  {
    pointerSRAM0Delay = pointerSRAM0-16*hdelay[channel];
    if(pointerSRAM0Delay < 0){pointerSRAM0Delay = numBytesInSRAM0 + pointerSRAM0Delay;}

    sram.readBuffer(pointerSRAM0Delay,ramBuffer,16); // read delayed sample. This will contain information from all channels
    for (int sramIndex = 0; sramIndex < 16; sramIndex+=2)
    {
      uint8_t chan = ((ramBuffer[sramIndex] & maskChannelOn) >> 5);//channel information is stored in the first 3 bits of the high byte
      if(chan == channel)//The channel stored in the sram byte matches the channel we are interested in for the PID control.
      {
        uint16_t hb = ((uint16_t) (ramBuffer[sramIndex] & maskChannelOff) << 8);
        uint16_t lb = ((uint16_t) ramBuffer[sramIndex+1]);
        VadcSRAM0[sramIndex/2] = (int) (hb | lb);
      }
    }
  }
  
    
  //Write into the SRAM buffer that is 1/loopsPerSRAM1Sample Hz. Look up time delayed values.
if(loopCounter % loopsPerSRAM1Sample == 0)
{
  //save the results to RAM, look-up time-delayed values to create a sampling interval longer than 60Hz period
  pointerSRAM1 = ((pointerSRAM1+16) % numBytesInSRAM1); // advance and wrap-around pointer
  sram.writeBuffer(pointerSRAM1,ramBuffer,16); // write these samples (from the previous loop sampling) to memory
  for (int channel = 0; channel < 8; channel++)
  {
    pointerSRAM1Delay = pointerSRAM1-16*hdelay[channel];
    if(pointerSRAM1Delay < 0){pointerSRAM1Delay = numBytesInSRAM1 + pointerSRAM1Delay;}

    sram.readBuffer(pointerSRAM1Delay,ramBuffer,16); // read delayed sample. This will contain information from all channels
    for (int sramIndex = 0; sramIndex < 16; sramIndex+=2)
    {
      uint8_t chan = ((ramBuffer[sramIndex] & maskChannelOn) >> 5);//channel information is stored in the first 3 bits of the high byte
      if(chan == channel)//The channel stored in the sram byte matches the channel we are interested in for the PID control.
      {
        uint16_t hb = ((uint16_t) (ramBuffer[sramIndex] & maskChannelOff) << 8);
        uint16_t lb = ((uint16_t) ramBuffer[sramIndex+1]);
        VadcSRAM1[sramIndex/2] = (int) (hb | lb);
      }
    }
  } 
}
  
}

void updatePIDParameters(){
     // compute the PID parameters for all 8 channels
   // calculate control parameters
   int16_t Vdiff = 0;
   for (int adc_chan=0; adc_chan<8; adc_chan++){
           Vdiff = Rslope[adc_chan] * (Vset[adc_chan]-Vadc[adc_chan]);
	   P[adc_chan]=K[adc_chan]*(b[adc_chan]*(Vdiff));  // proportional signal with
	   D[adc_chan]=ad[adc_chan]*D[adc_chan]-bd[adc_chan]*(Vadc[adc_chan]-VadcSRAM0[adc_chan]); // Differential gain with filtering
	   D[adc_chan] = 0;//CP 
           v[adc_chan]=P[adc_chan]+I[adc_chan]+D[adc_chan]; // idealized PID response
	   u[adc_chan]=max(min(v[adc_chan],UPSAT[adc_chan]),LOWSAT[adc_chan]); // saturated actuator simulation
           I[adc_chan]=I[adc_chan]+bi[adc_chan]*(Vdiff)+a0[adc_chan]*(u[adc_chan]-v[adc_chan]);  // Integrator signal with wind-up prevention
	   outphase[adc_chan]= (int) (maxDelay*(1-u[adc_chan]/(UPSAT[adc_chan]-LOWSAT[adc_chan])));  // maximizes dynamic range of PWM output
           outphase[adc_chan] = max(0,min(outphase[adc_chan],maxDelay));//added by CP to prevent negative outphases.
   }
}

void printDataThroughSerial(){
    int printChannel = NUFEREN_ADC_IN;
    
  //dio._openCommLink();
  //interlock_condition = dio.digitalRead(NUFERN_INTERLOCK_DIO_IN);
  //dio._closeCommLink();
  //Serial.println("interlock_condition");    
  //Serial.println(interlock_condition);    
    //Serial.print("RamPointer: ");
    //Serial.print(pointerSRAM0);
    /*
    Serial.print(" Vset: ");
    Serial.print(Vset[printChannel]);
    Serial.print(" Vadc: ");
    Serial.print(Vadc[printChannel]);
    Serial.print(" VadcSRAM0: ");
    Serial.print(VadcSRAM0[printChannel]);
    Serial.print(" v[printChannel]: ");
    Serial.print(v[printChannel]);
    /*Serial.print(" P: ");
    Serial.print(P[printChannel]);
    Serial.print(" I: ");
    Serial.print(I[printChannel]);
    Serial.print(" D: ");
    Serial.print(D[printChannel]);
    Serial.print(" u: ");
    Serial.println(u[printChannel]);
    Serial.print("\n\n outPhase: ");
    Serial.println(outphase[0]);
    Serial.print("\n");
    Serial.println(outphase[1]);
    Serial.print("\n");
    Serial.println(outphase[2]);
    Serial.print("\n");
    Serial.println(outphase[3]);
    Serial.print("\n");
    Serial.println(outphase[4]);
    Serial.print("\n");
    Serial.println(outphase[5]);
    Serial.print("\n");
    Serial.println(outphase[6]);
    Serial.print("\n");
    Serial.println(outphase[7]);
    /**/
    
    Serial.println("macadress: ");
    for(int i = 0; i < 6; i++){
    //Serial.println(mac[i]);
    //Serial.print(" ");
    }
    //Serial.println(" ");
    
    //Serial.println(maxDelay);
    //Serial.println(microsecondDelayfor220Phase);
    
    for(int i = 0; i < 8; i++){
    /*
    Serial.print("dioOutChannel: ");
    Serial.println(dioOutChannel[i]);
    Serial.print("enable: ");
    Serial.println(enable[i]);
    Serial.print("Rslope: ");
    Serial.println(Rslope[i]);
    Serial.print("Vset: ");
    Serial.println(Vset[i]);
    Serial.print("Ku: ");
    Serial.println(Ku[i]);
    Serial.print("Tu: ");
    Serial.println(Tu[i]);
    Serial.print("K: ");
    Serial.println(K[i]);
    Serial.print("Ti: ");
    Serial.println(Ti[i]);
    Serial.print("Td: ");
    Serial.println(Td[i]);
    Serial.print("Tt: ");
    Serial.println(Tt[i]);
    Serial.print("a0: ");
    Serial.println(a0[i]);
    Serial.print("bi: ");
    Serial.println(bi[i]);
    Serial.print("ad: ");
    Serial.println(ad[i]);
    Serial.print("bd: ");
    Serial.println(bd[i]);
    Serial.print("P: ");
    Serial.println(P[i]);
    Serial.print("I: ");
    Serial.println(I[i]);
    Serial.print("D: ");
    Serial.println(D[i]);
    Serial.print("u: ");
    Serial.println(u[i]);
    Serial.print("outphase: ");
    Serial.println(outphase[i]);*/
    //Serial.print("Vadc: ");
    //Serial.println(Vadc[i]);
    /**/
    //Serial.println();
    
    }
}

void sendReceiveDataByTCP(){
 //Ethernet
SPI.setDataMode(SPI_MODE0);//Needed for Ethernet chip.
if(!isSendingData)
{
  client = server.available();//Needs SPI_MODE0 (?) - Returns a Client Object
  if(client)
  {
    isSendingData = true;
  }
}

if(client)
{
  if(!client.connected())
  {
     if(USE_SERIAL){Serial.println("Error, not connected"); }
  }
  else
  {
   if(client.available())
   {
     if(readingTCP)
     {
       //Incoming POST Binary
       for(int i = 0 ; i < TCPBufferLength; i++)
       {
         uint8_t incomingByte = client.read();
         //TCPBuffer[TCPIndex] = incomingByte;
         if(USE_SERIAL)
         {
           Serial.println(incomingByte,BIN);
           Serial.println((char)incomingByte);
         }
         TCPIndex+=1;
       }
       TCPIndex = 0;
     }
     else
     {//writing to TCP
       client.print("POST HTTP/1.1\r\nHost: 10.1.1.200\r\nContent-Type: application/octet-stream\r\nContent-Length: ");
       client.print(TCPBufferLength);
       client.print("\r\n\r\n");
       writeToClient(client);
       client.stop();
       isSendingData = false; 
       delay(1);
     }
   }
   else
   {
     if(readingTCP)
     {
     client.print("HTTP/1.1 200 OK\r\n\r\n");//Needed to conclude POST/GET reading.
     client.stop();
     isSendingData = false; 
     delay(1);
     }
   }
  }
} 
}

void readFlowSwitches(){
  
dio._openCommLink();
switchOpen_CCW = (bool) dio.digitalRead(COLD_CITY_WATER_FLOW_SWITCH_TOTALIZER_DIO_IN);
switchOpen_RL = (bool) dio.digitalRead(RESERVOIR_LOOP_FLOW_SWITCH_TOTALIZER_DIO_IN);
dio._closeCommLink();

if(loopsOpen_CCW == 0 && switchOpen_CCW == true){
//Starting to count new cycle from the switch just going high
loopsOpen_CCW = 0;
loopsClosed_CCW = 0;
loopsOpen_CCW = loopsOpen_CCW + 1;
}

if(loopsOpen_RL == 0 && switchOpen_RL == true){
//Starting to count new cycle from the switch just going high
loopsOpen_RL = 0;
loopsClosed_RL = 0;
loopsOpen_RL = loopsOpen_RL + 1;
}

if(switchOpen_CCW == true){
loopsOpen_CCW = loopsOpen_CCW + 1;
}
if(switchOpen_RL == true){
loopsOpen_RL = loopsOpen_RL + 1;
}
if(switchOpen_CCW == false){
loopsClosed_CCW = loopsClosed_CCW + 1;
}
if(switchOpen_RL == false){
loopsClosed_RL = loopsClosed_RL + 1;
}

if(loopsOpen_CCW > 0 && loopsClosed_CCW > 0 && switchOpen_CCW == true){
//Starting to count new cycle from the switch just going high, but at the end
loopsClosed_CCW = loopsClosed_CCW - 1;
pulsesPerSecond_CCW = 60.0 / (loopsOpen_CCW + loopsClosed_CCW);
if(numFlowAverages_CCW >= totalNumFlowAverages){
  avePulsesPerSecond_CCW = (1.0*sumPulsesPerSecond_CCW)/(1.0*numFlowAverages_CCW);
  sumPulsesPerSecond_CCW = 0;
  numFlowAverages_CCW = 0;
}
sumPulsesPerSecond_CCW = sumPulsesPerSecond_CCW + pulsesPerSecond_CCW;
numFlowAverages_CCW = numFlowAverages_CCW + 1;

loopsOpen_CCW = 0;
loopsClosed_CCW = 0;
}

if(loopsOpen_RL > 0 && loopsClosed_RL > 0 && switchOpen_RL == true){
//Starting to count new cycle from the switch just going high, but at the end
loopsClosed_RL = loopsClosed_RL - 1;
pulsesPerSecond_RL = 60.0 / (loopsOpen_RL + loopsClosed_RL);
if(numFlowAverages_RL >= totalNumFlowAverages){
  avePulsesPerSecond_RL = (1.0*sumPulsesPerSecond_RL)/(1.0*numFlowAverages_RL);
  sumPulsesPerSecond_RL = 0;
  numFlowAverages_RL = 0;
}
sumPulsesPerSecond_RL = sumPulsesPerSecond_RL + pulsesPerSecond_RL;
numFlowAverages_RL = numFlowAverages_RL + 1;

loopsOpen_RL = 0;
loopsClosed_RL = 0;
}

}

void checkInterlock(){
  dio._openCommLink();
  interlock_condition = (bool) dio.digitalRead(NUFERN_INTERLOCK_DIO_IN);
  dio._closeCommLink();
  if(interlock_condition == true)//false means that water is flowing (Or that the Nufern is off)
  {
    //No water is flowing. Disable;
    if(USE_SERIAL){Serial.println("interlocked, disabled. No Flow");}
    enable[NUFEREN_ADC_IN] = HIGH_MCP;
  } 
  else//Water is Flowing. (Nufern Interlock flow switch is closed when flowing)
  {
    if(USE_SERIAL){Serial.println("Water Flowing");}
    enable[NUFEREN_ADC_IN] = LOW_MCP;
    
      //Check if Heat is too High
    if( errVoltageTomKConversion[NUFEREN_ADC_IN]*(Vset[NUFEREN_ADC_IN]-Vadc[NUFEREN_ADC_IN]) > 500)//false means that water is flowing
    {  //Nufern is a Degree Too Hot. Disable;
      if(USE_SERIAL){Serial.println("interlocked, disabled. Too Hot");}
      enable[NUFEREN_ADC_IN] = HIGH_MCP;
    } 
  }
  if(USE_SERIAL){
    Serial.print("enable[NUFEREN_ADC_IN] = ");
    Serial.println(enable[NUFEREN_ADC_IN]);
  }
}

void loop(){
//if(USE_SERIAL){Serial.println("loop beginning");}
//Begin Section that ends when low.
//int count = 0;
uint8_t value = 0x00;
dio._openCommLink();     
// Sets details necessary to transfer bits to/from the chip. Must call before any set of SPI commumications.
while (!((bool)(value & dio.pinForFastRead)))//loops while low (high when triggering off of "TRIGB" from ribbon.
{
  PORTB &= 0b11111101;                        // Direct port manipulation speeds taking Slave Select LOW before SPI action. Take the slave-select pin HIGH to avoid conflicts. This is equivalent to the previous line.
  SPI.transfer(OPCODER_MCP | (dio.address << 1));      // Send the MCP23S17 opcode, chip address, and read bit
  SPI.transfer(GPIOB_MCP);                              // Send the register we want to read
  value = SPI.transfer(0x00);                           // Send any byte, the function will return the read value (register address pointer will auto-increment after write)
  PORTB |= 0b00000010;
}
 
 //outphase[NUFEREN_ADC_IN] = 680;
 
 delayMicroseconds(microsecondDelayfor220Phase);//FIX ME Hack to make the nufern 220 back in phase will screw up all other triacs....


 
//This section begins on line trigger voltage high.

//Initial scr triggering is right before the LINE_TRIGGER goes low.scr
for (uint16_t timer=0; timer <= maxDelay; timer++){
	for (int adc_chan = 0; adc_chan<8; adc_chan++){
		if (timer == outphase[adc_chan]){
  			dio.digitalWrite(dioOutChannel[adc_chan], enable[adc_chan]); //Note it is important to take into account the phase of the line voltage relative to the line trigger.
  			if(adc_chan != RESERVOIR_ADC_IN){//Reservoir output stays low for the rest of the half cycle.
                        dio.digitalWrite(dioOutChannel[adc_chan], HIGH_MCP); 
                        }
		}
	}
  }
  
for (int adc_chan=0; adc_chan<8; adc_chan++){
    dio.digitalWrite(dioOutChannel[adc_chan],HIGH_MCP);
}
//HERE: --> Loop ends close to the beginning of when LINE_TRIGGER goes low.



readInAnalogInputs();
saveInputsToRam(loopCounter);
updatePIDParameters();
readFlowSwitches();

checkInterlock();

if(USE_SERIAL && loopCounter%(500) ==0){
  printDataThroughSerial(); 
}


sendReceiveDataByTCP();

loopCounter = loopCounter + 1;
//if(USE_SERIAL){Serial.println(freeRam());}
/**/
}

