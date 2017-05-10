//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ntAVRnet.h"
#include "timer.h"
#include "ethernet.h"
#include "arp.h"
#include "tcp.h"
#include "dhcp.h"
//----------------------------------------------------------------------------
extern volatile unsigned int time_watchdog;
static volatile unsigned long UptimeMs;
static volatile unsigned char Counter10ms;
static volatile unsigned int Counter1s;
//----------------------------------------------------------------------------
void timer1Init(void)
{
	// initialize timer 1
	// set prescaler on timer 1
	TCCR1B = (TCCR1B & ~TIMER_PRESCALE_MASK) | TIMER1PRESCALE;	// set prescaler
	TCNT1H = 0;						// reset TCNT1
	TCNT1L = 0;
	TIMSK |= (1<<TOIE1);						// enable TCNT1 overflow
	TCNT1 = 0xFFFF - TIMER1_INTERVAL;
}
void timerInit(void)
{
	timer1Init();
	sei();
}
//! Interrupt handler for tcnt1 overflow interrupt
TIMER_INTERRUPT_HANDLER(SIG_OVERFLOW1)
{
	//Tai nap gia tri timer 1
	TCNT1 = 0xFFFF - TIMER1_INTERVAL;
	//Cap nhat watchdog timer
	if((time_watchdog++) > 120){
        time_watchdog = 0;
        ethInit();
	}
	Counter1s++;
	arpTimer();
	TCPCheckTimeOut();////
	dhcpTimer();////
}
