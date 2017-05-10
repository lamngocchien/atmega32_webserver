//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef TIMER_H
#define TIMER_H
//----------------------------------------------------------------------------
#define TIMER_CLK_STOP			0x00	///< Timer Stopped
#define TIMER_CLK_DIV1			0x01	///< Timer clocked at F_CPU
#define TIMER_CLK_DIV8			0x02	///< Timer clocked at F_CPU/8
#define TIMER_CLK_DIV64			0x03	///< Timer clocked at F_CPU/64
#define TIMER_CLK_DIV256		0x04	///< Timer clocked at F_CPU/256
#define TIMER_CLK_DIV1024		0x05	///< Timer clocked at F_CPU/1024
#define TIMER_CLK_T_FALL		0x06	///< Timer clocked at T falling edge
#define TIMER_CLK_T_RISE		0x07	///< Timer clocked at T rising edge
#define TIMER_PRESCALE_MASK		0x07	///< Timer Prescaler Bit-Mask

#define TIMER1PRESCALE		TIMER_CLK_DIV1024		///< timer 1 prescaler default

#ifndef TIMER_INTERRUPT_HANDLER
#define TIMER_INTERRUPT_HANDLER		SIGNAL
#endif
void timer1Init(void);
void timerInit(void);


#endif //TIMER_H
