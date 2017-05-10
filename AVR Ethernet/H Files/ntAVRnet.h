//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef NTAVRNET_H
#define NTAVRNET_H

#ifndef F_CPU
	#define F_CPU        12000000	// Cho toc do la 12MHz
#endif //F_CPU
#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	//So chu ky lenh trong 1 micro giay

#define LO8(x) ((x)&0xFF)
#define HI8(x) (((x)>>8)&0xFF)

#define UART_BAUDRATE	9600l

#define TIMER_PRESCALE		1024
#define TIMER1_INTERVAL		(F_CPU/TIMER_PRESCALE)

#define IPDOT(a,b,c,d)	((unsigned long)((unsigned char)a)<<24)+((unsigned long)((unsigned char)b)<<16)+((unsigned long)((unsigned char)c)<<8)+(unsigned char)d//((a<<24)|(b<<16)|(c<<8)|(d))
//#define IPADDRESS			IPDOT(192,168,1,10)
#define IPADDRESS			IPDOT(0,0,0,0)
#define NETMASK				IPDOT(0,0,0,0)
#define GATEWAY				IPDOT(0,0,0,0)

#define ETHADDR0	'0'
#define ETHADDR1	'F'
#define ETHADDR2	'F'
#define ETHADDR3	'I'
#define ETHADDR4	'C'
#define ETHADDR5	'E'
//
#define HTTP_AUTH_STRING "admin:1234"
//
#endif //NTAVRNET_H
