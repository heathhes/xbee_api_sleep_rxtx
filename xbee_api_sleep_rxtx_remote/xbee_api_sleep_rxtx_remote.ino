/*This code transmits a data payload through the xbee
  in API mode. It sleeps inbetween transmissions and is 
  wakened by the xbee sleep-off output on an interrupt pin
  
*/

//-----------------
#include <SoftwareSerial.h>
#include "LowPower.h"
#include "setup.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS A3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//-----address of destination-----
                            //for broadcast
int8_t ADDR_B1 = 0x00;      //0x00
int8_t ADDR_B2 = 0x13;      //0x00
int8_t ADDR_B3 = 0xA2;      //0x00
int8_t ADDR_B4 = 0x00;      //0x00
int8_t ADDR_B5 = 0x41;      //0x00
int8_t ADDR_B6 = 0x25;      //0x00
int8_t ADDR_B7 = 0xA4;      //0xFF
int8_t ADDR_B8 = 0x79;      //0xFF


SoftwareSerial softSerial(7,8);  //(rx,tx)

uint8_t tx_array[] = {0x7E,0x00,0x13,0x10,0x01,
                    ADDR_B1,ADDR_B2,ADDR_B3,
                    ADDR_B4,ADDR_B5,ADDR_B6,
                    ADDR_B7,ADDR_B8,
                    0xFF,0xFE,0x00,0x00,0x31,
                    0x22,0x23,0x24,0x25,0xB6};

uint8_t rx_array[21];                    
int rx_count = 0;
const int led_pin = 13;  
const int wake_pin = 3;
const int relay_pin = 9;

uint8_t payload[] = {0x99,0x99,0x99,0x99,0x99};

boolean i = 0;
void wakeUp() 
{  
  //called after interrupt // no delays or millis
  rx_count++;
}  
  
void setup() 
{  
  //turn on relay to energize sensors for initial configs
  pinMode(relay_pin,OUTPUT); digitalWrite(relay_pin, HIGH);
  delay(100);
  
  // Start up the library for dallas temp
  sensors.begin();
   
  softSerial.begin(9600);
  Serial.begin(9600);
  softSerial.println("xbee_interrupt_sleep_txrx");
  pinMode(wake_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(relay_pin,OUTPUT); digitalWrite(relay_pin,LOW); 
  digitalWrite(led_pin, HIGH);
  delay(1000);
  digitalWrite(led_pin, LOW); 

   setDestination(xbee1);     //set initial destination
}  
  
void loop() 
{ 
  attachInterrupt(1, wakeUp, RISING);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(1);  
  delay(250);     //delay after wakeup
  mainFunction(); 
} 

void mainFunction()
{
  digitalWrite(led_pin, HIGH);

  //turn on the relay to power sensors
  digitalWrite(relay_pin, HIGH);
  delay(100);
  
  //read sensors and create payload for tx
  payload[0] = getAnalog8bit(A0,5,5); //battery
  payload[1] = getAnalog8bit(A1,5,5); //light  
  payload[2] = getDallasTemp(); //temp

  //turn off relay to power down sensors
  delay(100);
  //digitalWrite(relay_pin,LOW);

  //insert payload into tx array
  for(int i = 0; i < (sizeof(payload)); i++)
  {
    tx_array[17+i] = payload[i];
  } 

  //-----create checksum byte for transmission-----
  int8_t check_sum = getTxCheckSum();
  tx_array[22] = check_sum;

  //-----transmit meta-data and payload-----
  for(int i = 0; i < 24; i++)
  {
    Serial.write(tx_array[i]);
  }
  digitalWrite(led_pin, LOW);
  //delay(1000);

  getRxData();
  
}


void getRxData()
{
  while(Serial.available())
  {
    if(Serial.read() == 0x7E)
    {
      rx_array[0] = 0x7E;
      for(int i = 1; i < 21; i++)
        rx_array[i] = Serial.read();
    }
  }

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

  //Serial.flush();
  delay(100);

  //clear array
  for(int i = 0; i < 21; i++)
    rx_array[i] = 0x00;
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

boolean setDestination(int xbee)
{
  switch(xbee){
    case 0: 
        insertAddress(xbee_all_address);
        break;  
    case 1: 
        insertAddress(xbee1_address);
        break;
    case 2: 
        insertAddress(xbee2_address);
        break;
    case 3: 
        insertAddress(xbee3_address);
        break;
    case 4: 
        insertAddress(xbee4_address);
        break;
    case 5: 
        insertAddress(xbee5_address);
        break;
    case 6: 
        insertAddress(xbee6_address);
        break;                
  }
}

void insertAddress(uint8_t address[8])
{
  for(int i = 0; i < 8; i++)
  {
    tx_array[5+ i] = address[i];
  } 
}

int8_t getTxCheckSum()
{
  long sum = 0;
  for(int i = 3; i < (sizeof(tx_array) - 1); i++)
  {
    sum += tx_array[i]; 
  } 
  int8_t check_sum = 0xFF - (sum & 0xFF);
  return check_sum; 
}

uint8_t getAnalog8bit(uint8_t pin, uint8_t samples, uint8_t dly)
{
  analogRead(pin);
  delay(50);
  int dac_value = 0;
  for(int i = 0; i < samples; i++)
  {
    dac_value += analogRead(pin);
    delay(dly);
  }
  
  dac_value /= samples;
  dac_value /= 4;       //to return and 8 byte value (0-255);
  return dac_value;
}

uint8_t getDallasTemp()
{
  delay(10);
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println(sensors.getTempCByIndex(0));
  float tempC = sensors.getTempCByIndex(0);
  //float tempF = (sensors.getTempCByIndex(0) * 9.0 / 5.0) + 32;
  //Serial.println(tempC);
  //Serial.println(tempF);

  uint8_t temp2dac = tempC * 4; //can only transmit byte so save resolution
  
  return temp2dac;
}
