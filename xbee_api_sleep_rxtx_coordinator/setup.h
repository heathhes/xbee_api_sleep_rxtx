/*
  setup.h - Library for setting up xbee network and 328 base pcb
  interrupt pin, baudrate, software serial pins
*/
#ifndef setup_h
#define setup_h

extern uint8_t xbee_all = 0;
extern uint8_t xbee1 = 1;
extern uint8_t xbee2 = 2;
extern uint8_t xbee3 = 3;
extern uint8_t xbee4 = 4;
extern uint8_t xbee5 = 5;
extern uint8_t xbee6 = 6;

extern uint8_t xbee_all_address[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF};
extern uint8_t xbee1_address[8] = {0x00,0x13,0xA2,0x00,0x41,0x25,0xA4,0x79}; //sleep coordinator
extern uint8_t xbee2_address[8] = {0x00,0x13,0xA2,0x00,0x41,0x25,0xA4,0x81};
extern uint8_t xbee3_address[8] = {0x00,0x13,0xA2,0x00,0x41,0x4E,0x65,0x93};
extern uint8_t xbee4_address[8] = {0x00,0x13,0xA2,0x00,0x41,0x4E,0x65,0x8D};
extern uint8_t xbee5_address[8] = {0x00,0x13,0xA2,0x00,0x41,0x4E,0x65,0x8E};
extern uint8_t xbee6_address[8] = {0x00,0x13,0xA2,0x00,0x41,0x25,0xA4,0x95};

extern uint8_t xbee1_data_in[5] = {0,0,0,0,0};
extern uint8_t xbee2_data_in[5] = {0,0,0,0,0};
extern uint8_t xbee3_data_in[5] = {0,0,0,0,0};
extern uint8_t xbee4_data_in[5] = {0,0,0,0,0};
extern uint8_t xbee5_data_in[5] = {0,0,0,0,0};
extern uint8_t xbee6_data_in[5] = {0,0,0,0,0};

extern uint8_t xbee1_data_out[5] = {0,0,0,0,0};
extern uint8_t xbee2_data_out[5] = {0,0,0,0,0};
extern uint8_t xbee3_data_out[5] = {0,0,0,0,0};
extern uint8_t xbee4_data_out[5] = {0,0,0,0,0};
extern uint8_t xbee5_data_out[5] = {0,0,0,0,0};
extern uint8_t xbee6_data_out[5] = {0,0,0,0,0};


extern uint8_t battery = 0;
extern uint8_t light_sensor = 1;
extern uint8_t temp_sensor = 3;

extern float battery2 = 0;
extern float light2 = 0;
extern float temp2 = 0;

extern float battery3 = 0;
extern float light3 = 0;
extern float temp3 = 0;


#endif
