/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef NTAVRNET_H
#define NTAVRNET_H
#include <avr/pgmspace.h>
#ifndef F_CPU
	#define F_CPU        8000000	// Cho toc do la 12MHz
#endif //F_CPU
#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	//So chu ky lenh trong 1 micro giay

#define LO8(x) ((x)&0xFF)
#define HI8(x) (((x)>>8)&0xFF)

#define UART_BAUDRATE	9600l

#define TIMER_PRESCALE		1024
#define TIMER1_INTERVAL		(F_CPU/TIMER_PRESCALE)

#define IPDOT(a,b,c,d)	((unsigned long)((unsigned char)a)<<24)+((unsigned long)((unsigned char)b)<<16)+((unsigned long)((unsigned char)c)<<8)+(unsigned char)d//((a<<24)|(b<<16)|(c<<8)|(d))

//Define IP
#define IPADDRESS			IPDOT(0,0,0,0)
#define NETMASK				IPDOT(0,0,0,0)
#define GATEWAY				IPDOT(0,0,0,0)

//Define  MAC
#define ETHADDR0	'0'
#define ETHADDR1	'O'
#define ETHADDR2	'F'
#define ETHADDR3	'I'
#define ETHADDR4	'C'
#define ETHADDR5	'E'

//Define Device
#define setbit(port,pin) port |= 1<<pin 
#define clrbit(port,pin) port &= ~(1<<pin)
#define ON1    clrbit(PORTC,PORTC7);
#define ON2    clrbit(PORTC,PORTC6);
#define ON3    clrbit(PORTC,PORTC5);
#define ON4    clrbit(PORTC,PORTC4);
#define ON_ALARM	clrbit(PORTC,PORTC3);
#define OFF1    setbit(PORTC,PORTC7);
#define OFF2    setbit(PORTC,PORTC6);
#define OFF3    setbit(PORTC,PORTC5);
#define OFF4    setbit(PORTC,PORTC4);
#define OFF_ALARM	setbit(PORTC,PORTC3);
#define AUTO1	begin_auto(1);
#define AUTO2	begin_auto(2);
#define AUTO3	begin_auto(3);
#define AUTO4	begin_auto(4);

#define ALARM 80


//Define Password HTTP Server
#define HTTP_AUTH_STRING "admin:1234"

//Define Ham Con
uint8_t GetState(uint8_t val);
void showclock (void);
void showtemp (void);
void set_time_count(uint8_t device, uint8_t state, uint16_t time);
void set_temp_val(unsigned char temp, unsigned char state, unsigned char device);
void reset_one_device(uint8_t device);
void reset_all(void);
unsigned char test_out_state(unsigned char device);
void edit_clock(uint8_t type, uint8_t val1, uint8_t val2, uint8_t val3);
void edit_day(uint8_t day);
#endif //NTAVRNET_H
