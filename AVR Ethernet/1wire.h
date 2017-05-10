/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */ 
#include <util/delay.h>
#ifndef F_CPU
	#define F_CPU        8000000UL	// Cho toc do la 8MHz
#endif //F_CPU
#ifndef MY1WIRE_H_
#define MY1WIRE_H_
//#define THERM_DECIMAL_STEPS_12BIT 625 //.0625 
//#define THERM_DECIMAL_STEPS_9BIT 500 //.500
#define THERM_CMD_CONVERTTEMP 0x44 
#define THERM_CMD_RSCRATCHPAD 0xbe 
#define THERM_CMD_WSCRATCHPAD 0x4e 
#define THERM_CMD_CPYSCRATCHPAD 0x48 
#define THERM_CMD_RECEEPROM 0xb8 
#define THERM_CMD_RPWRSUPPLY 0xb4 
#define THERM_CMD_SEARCHROM 0xf0 
#define THERM_CMD_READROM 0x33 
#define THERM_CMD_MATCHROM 0x55 
#define THERM_CMD_SKIPROM 0xcc 
#define THERM_CMD_ALARMSEARCH 0xec 

#define THERM_PORT PORTB
#define THERM_DDR DDRB
#define THERM_PIN PINB
#define THERM_DQ PB0

#define THERM_INPUT_MODE() THERM_DDR&=~(1<<THERM_DQ) 
#define THERM_OUTPUT_MODE() THERM_DDR|=(1<<THERM_DQ) 

#define THERM_LOW() THERM_PORT&=~(1<<THERM_DQ) 
#define THERM_HIGH() THERM_PORT|=(1<<THERM_DQ)  
uint8_t therm_reset() {
        uint8_t i;
        // Pull line low and wait for 480uS
        THERM_LOW();
        THERM_OUTPUT_MODE();
        _delay_loop_2(960);
		
        //Release line and wait for 60uS
        THERM_INPUT_MODE();
        _delay_loop_1(160);
		
        //Store line value and wait until the completion of 480uS period
        i=(THERM_PIN & (1<<THERM_DQ));
        _delay_loop_2(840);
        //Return the value read from the presence pulse (0=OK, 1=WRONG)
        return i;
}
 
void therm_write_bit(uint8_t bit){
        //Pull line low for 1uS
        THERM_LOW();
        THERM_OUTPUT_MODE();
        _delay_loop_1(3);
		
        //If we want to write 1, release the line (if not will keep low)
        if(bit) THERM_INPUT_MODE();
		
        //Wait for 60uS and release the line
        _delay_loop_1(160);
        THERM_INPUT_MODE();
}
 
uint8_t therm_read_bit(void){
        uint8_t bit=0;
		
        //Pull line low for 1uS
        THERM_LOW();
        THERM_OUTPUT_MODE();
        _delay_loop_1(3);
 
        //Release line and wait for 14uS
        THERM_INPUT_MODE();
        _delay_loop_1(37);
 
        //Read line value
        if(THERM_PIN&(1<<THERM_DQ)) bit=1;
        //Wait for 45uS to end and return read value
        _delay_loop_1(120);
        return bit;
}
 
uint8_t therm_read_byte(void){
        uint8_t i=8, n=0;
        while(i--){
                //Shift one position right and store read value
                n>>=1;
                n|=(therm_read_bit()<<7);
        }
        return n;
}
 
void therm_write_byte(uint8_t byte){
        uint8_t i=8;
        while(i--){
                //Write actual bit and shift one position right to make the next bit ready
                therm_write_bit(byte&1);
                byte>>=1;
        }
}
 
//***********************************************************************************************//
//Do chinh xac 0.0625
//Nhiet do return gap x lan gia tri thuc do duoc
void therm_read_temperature()
{ 
	// Buffer length must be at least 12bytes long! ["+XXX.XXXX C"
	extern uint8_t ds18b20, digit;
	uint8_t temperature[2]; 
	//Reset, skip ROM and start temperature conversion 
	therm_reset(); 
	therm_write_byte(THERM_CMD_SKIPROM); 
	therm_write_byte(THERM_CMD_CONVERTTEMP); 
	//Wait until conversion is complete 
	while(!therm_read_bit()); 
	//Reset, skip ROM and send command to read Scratchpad 
	therm_reset(); 
	therm_write_byte(THERM_CMD_SKIPROM);
	therm_write_byte(THERM_CMD_RSCRATCHPAD); 
	//Read Scratchpad (only 2 first bytes) 
	temperature[0]=therm_read_byte(); 
	temperature[1]=therm_read_byte(); 
	therm_reset(); 
	//Store temperature integer digits and decimal digits 
	digit  = ((float)(temperature[0]&0x0f))/16*100;
	ds18b20 = ((temperature[0]&0xf0)>>4)|((temperature[1]&0x0f)<<4);
}

////Do chinh xac 1
//unsigned int therm_read_temperature()
//{ 
	//// Buffer length must be at least 12bytes long! ["+XXX.XXXX C"] 
	//uint8_t temperature[2]; 
	////Reset, skip ROM and start temperature conversion 
	//therm_reset(); 
	//therm_write_byte(THERM_CMD_SKIPROM); 
	//therm_write_byte(THERM_CMD_CONVERTTEMP); 
	////Wait until conversion is complete 
	//while(!therm_read_bit()); 
	////Reset, skip ROM and send command to read Scratchpad 
	//therm_reset(); 
	//therm_write_byte(THERM_CMD_SKIPROM);
	//therm_write_byte(THERM_CMD_RSCRATCHPAD); 
	////Read Scratchpad (only 2 first bytes) 
	//temperature[0]=therm_read_byte(); 
	//temperature[1]=therm_read_byte(); 
	//therm_reset(); 
	////Store temperature integer digits and decimal digits 
	//digit=temperature[0]>>4; 
	//digit|=(temperature[1]&0x7)<<4;
	//return (digit);
//}
uint8_t config_ds18b20(uint8_t H,uint8_t L,uint8_t mode)
{
	therm_reset(); 
	therm_write_byte(0x4E);
	//therm_write_byte(0xFF);
	//therm_write_byte(0xFF);
	//therm_write_byte(0x1F);
	therm_write_byte(H);
	therm_write_byte(L);
	therm_write_byte(mode);	
}
//unsigned int therm_read_temperature()
//{ 
	//// Buffer length must be at least 12bytes long! ["+XXX.XXXX C"] 
	//uint8_t temperature[2]; 
	////Reset, skip ROM and start temperature conversion 
	//therm_reset(); 
	//therm_write_byte(THERM_CMD_SKIPROM); 
	//therm_write_byte(THERM_CMD_CONVERTTEMP); 
	////Wait until conversion is complete 
	//while(!therm_read_bit()); 
	////Reset, skip ROM and send command to read Scratchpad 
	//therm_reset(); 
	//therm_write_byte(THERM_CMD_SKIPROM);
	//therm_write_byte(THERM_CMD_RSCRATCHPAD); 
	////Read Scratchpad (only 2 first bytes) 
	//temperature[0]=therm_read_byte(); 
	//temperature[1]=therm_read_byte(); 
	//therm_reset(); 
	////Store temperature integer digits and decimal digits 
	//digit=temperature[0]>>3; 
	//digit|=(temperature[1]&0x7)<<5;
	//digit=digit>>1;
	////Hieu chinh nhiet
	//digit-=3;
	//return (digit);
//}

#endif /* MY1WIRE_H_ */

