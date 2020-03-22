
/*This code transmits a data payload through the xbee
  in API mode. It sleeps inbetween transmissions and is 
  wakened by the xbee sleep-off output on an interrupt pin
  
*/
#define BLYNK_PRINT Serial

#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>

#include <BlynkSimpleEthernet.h>
#include "LowPower.h"
#include "setup.h"
#include "SimpleTimer.h"

char auth[] = "ed2b10f6ec7a46b1aef3c363778c5973";


#define W5100_CS  10
#define SDCARD_CS 4

//-----address of destination-----
                            //for broadcast
int8_t ADDR_B1 = 0x00;      //0x00
int8_t ADDR_B2 = 0x13;      //0x00
int8_t ADDR_B3 = 0xA2;      //0x00
int8_t ADDR_B4 = 0x00;      //0x00
int8_t ADDR_B5 = 0x41;      //0x00
int8_t ADDR_B6 = 0x25;      //0x00
int8_t ADDR_B7 = 0xA4;      //0xFF
int8_t ADDR_B8 = 0x81;      //0xFF


SoftwareSerial softSerial(7,8);  //(rx,tx)

uint8_t tx_array[] = {0x7E,0x00,0x13,0x10,0x01,
                    ADDR_B1,ADDR_B2,ADDR_B3,
                    ADDR_B4,ADDR_B5,ADDR_B6,
                    ADDR_B7,ADDR_B8,
                    0xFF,0xFE,0x00,0x00,0x11,
                    0x22,0x33,0x44,0x55,0xB6};
                    
const int tx_array_size = 23;
                    
uint8_t rx_array[21];
const int led_pin = 13;  
const int wake_pin = 3;
const int msg_size = 21;

SimpleTimer sendTimer;

boolean new_data = true;

BlynkTimer timer;


////////////////////////////////////////
//Get Value from blynk V10  
BLYNK_WRITE(V10)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  //Serial.print("V1 Slider value is: ");
  if(pinValue)
  {
    digitalWrite(led_pin, HIGH);
    tx_array[18] = 0x21;
  }

  else
  {
    digitalWrite(led_pin,LOW);
    tx_array[18] = 0x20;
  }

  
}

////////////////////////////////////////
//Setup loop  
void setup() {  

  sendTimer.setInterval(1000,sendTxData);
  
  //initialize array to 0's
  for(int i = 0; i < msg_size; i++)
    rx_array[i] = 0;

  //set up serial for xbee and uart
  softSerial.begin(9600);
  Serial.begin(9600);
  Serial.println("xbee_api_coordinator_txrx");
  
  //pin declarations
  pinMode(wake_pin, INPUT);
  pinMode(led_pin, OUTPUT); 
  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card

  //setup blynk parameters
  Blynk.begin(auth);
  timer.setInterval(5000,timerPostData);
}  

  
////////////////////////////////////////
//Main loop
void loop() 
{ 

  //Receive any transmitted data
  int xbee = getRxData();

  if(new_data)
  {
    //sendTxData();
    //printTxData();
    
    //sendTxData();
    //Serial.println(xbee);
    convertSensorData(xbee); //convert the dac units to sensor values
    
  }
  new_data = false;

  sendTimer.run();
  //sendTxData();
  //printTxData();
  
  //post the data to Blynk
  Blynk.run();
  timer.run();

  //getTxData();

  //sendTxData();

  delay(100);
} 


////////////////////////////////////////
//Send tx data
void sendTxData()
{
  
  //-----create checksum byte for transmission-----
  int8_t check_sum = getTxCheckSum();
  tx_array[22] = check_sum;

  //-----transmit meta-data and payload-----
  for(int i = 0; i < 24; i++)
    Serial.write(tx_array[i]);
  
}


////////////////////////////////////////
//Print out the received array
void printTxData()
{
  for(int i = 0; i < 23; i++)
  {
    softSerial.print(tx_array[i],HEX);
    softSerial.print(", ");
  }
  softSerial.println();
}

////////////////////////////////////////
//Get the received data via xbee, verify integrity, 
//determine the tx xbee, extract payload
int getRxData()
{

//  if(Serial.available() >= 21)
//  {
//    for(int i = 0; i < 21; i++){
//      rx_array[i] = Serial.read();
//    }
//  }

  while(Serial.available())
  {
    if(Serial.read() == 0x7E)
    {
      rx_array[0] = 0x7E;
      for(int i = 1; i < 21; i++)
        rx_array[i] = Serial.read();
    }
  }
  
  int rx_source = 0;
  
  //if the first byte is 0x7E
  if(rx_array[0] == 0x7E)
  {
    new_data = true;
    
    //printRxArray();

    //verify the checksum
    if(rx_array[20] == getRxCheckSum())
    {
      for(int i = 0; i < 21; i++)
      {
        softSerial.print(rx_array[i],HEX);
        softSerial.print(", ");
      } 
      softSerial.println(); 
    }
    
    //determine source
    rx_source = getAddress(rx_array[11]); //11 byte is last address byte

    //store payload from source
    switch(rx_source)
    {
      case 2:
        for(int i = 0; i < 5; i++)
          xbee2_data_in[i] = rx_array[i+15]; //15th byte is beginning of payload
        //Serial.print("xbee2: ");
        //printDataArray(xbee2_data_in);
        break;

      case 3:
        for(int i = 0; i < 5; i++)
          xbee3_data_in[i] = rx_array[i+15]; //15th byte is beginning of payload     
        //Serial.print("xbee3: ");
        //printDataArray(xbee3_data_in);
        break;

      case 4:
        for(int i = 0; i < 5; i++)
          xbee4_data_in[i] = rx_array[i+15]; //15th byte is beginning of payload
        printDataArray(xbee4_data_in);
        break;

      case 5:
        for(int i = 0; i < 5; i++)
          xbee5_data_in[i] = rx_array[i+15]; //15th byte is beginning of payload       
        printDataArray(xbee5_data_in);
        break; 
        
      case 6:
        for(int i = 0; i < 5; i++)
          xbee6_data_in[i] = rx_array[i+15]; //15th byte is beginning of payload
        printDataArray(xbee6_data_in);
        break;   

      default:
        break;

        
    }

    //Clear the recieve array for next transmission
    clearRxArray();
    
    return rx_source;

  }
 
}


////////////////////////////////////////
//Get the address of the received transmission
int getAddress(int address_byte)
{ 
  // test each element to be the same. if not, return false
  int xbee = 0;
  switch(address_byte)
  {
    case 0x79:  
      xbee = 1;
      break;
    case 0x81:
      xbee = 2;
      break;
    case 0x93:
      xbee = 3;
      break;
    case 0x8D:
      xbee = 4;
      break;
    case 0x8E:
      xbee = 5;
      break;
    case 0x95:
      xbee = 6;
      break;
    default:
      xbee = 0;
      break;
  }  
  return xbee;
}


  
////////////////////////////////////////
//Convert 
void convertSensorData(int xbee)
{

  switch(xbee)
  {
    case 0:
      break;

    case 1:
      break;
      
    case 2:   
      battery2 = getSensorValue(1,xbee2_data_in[0]); //battery
      light2 = getSensorValue(2,xbee2_data_in[1]); //light sensor
      temp2 = getSensorValue(3,xbee2_data_in[2]); //temp sensor
      break;
    case 3:
      battery3 = getSensorValue(1,xbee3_data_in[0]); //battery
      light3 = getSensorValue(2,xbee3_data_in[1]); //light sensor
      temp3 = getSensorValue(3,xbee3_data_in[2]); //temp sensor
      break;
    default:
      break;
  } 
}


////////////////////////////////////////
//Convert dac units to sensor values
float getSensorValue(int type, float dac_value)
{
  float sensor_value = 0;
  switch(type)
  {
    //no sensor, do nothing
    case 0:
      break;
    
    //battery from A0 
    case 1:
      sensor_value = (dac_value/255)*2*3.3;
      break;
      
    //light sensor, logarithmic
    case 2:
      sensor_value = map(dac_value,0,220,0,100);
      break;
      
    //temp sensor, Dallas/Maxim DS18B20
    case 3:
    {
      float tempC = dac_value/4; // tx value was multiplied by 4 for resolution
      sensor_value = (tempC * 9.0 / 5.0) + 32;
    }
      break;
      
    default:
      sensor_value = 0;
      break;
  }

  return sensor_value;
}


////////////////////////////////////////
//Write data to blynk
void timerPostData()
{
  //if new data is good for this device, post
    //else post error (xbee # and type error)
    
  //xbee2
  Blynk.virtualWrite(V0,battery2);
  Blynk.virtualWrite(V1,light2);
  Blynk.virtualWrite(V2,temp2);

  //xbee3
  Blynk.virtualWrite(V3,battery3);
  Blynk.virtualWrite(V4,light3);
  Blynk.virtualWrite(V5,temp3);
  
}


////////////////////////////////////////
//Print out the received array
void printRxArray()
{
  for(int i = 0; i < 21; i++)
  {
    softSerial.print(rx_array[i],HEX);
    softSerial.print(", ");
  }
  softSerial.println();
}


////////////////////////////////////////
//Print out the data array
void printDataArray(uint8_t array[])
{
   for(int i = 0; i < 5; i++)
   {
     softSerial.print(array[i]);
     softSerial.print(", ");
   }
   softSerial.println();
}


////////////////////////////////////////
//Clear the received array
void clearRxArray()
{
  for(int i = 0; i < 21; i++){
    rx_array[i] = 0;
  }
}


////////////////////////////////////////
//Get the check sum of recieved message
int8_t getRxCheckSum(){
  long sum = 0;
  for(int i = 3; i < (sizeof(rx_array) - 1); i++){
    sum += rx_array[i]; 
  } 
  int8_t check_sum = 0xFF - (sum & 0xFF);
  return check_sum; 
}

////////////////////////////////////////
//Get the check sum of recieved message
int8_t getTxCheckSum(){
  long sum = 0;
  for(int i = 3; i < (sizeof(tx_array) - 1); i++){
    sum += tx_array[i]; 
  } 
  int8_t check_sum = 0xFF - (sum & 0xFF);
  return check_sum; 
}
