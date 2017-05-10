//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdarg.h>
#include "ntAVRnet.h"
#include "uart.h"
//----------------------------------------------------------------------------
char UartRxBuffer[UART_RX_BUFFER_SIZE];
char UartTxBuffer[UART_TX_BUFFER_SIZE];
volatile unsigned char UartTxBufferStart;
volatile unsigned char UartTxBufferLen;
volatile unsigned char UartRxBufferStart;
volatile unsigned char UartRxBufferLen;
static char HexTable[] PROGMEM= "0123456789ABCDEF";
//----------------------------------------------------------------------------
void uartInit(unsigned long baudrate)
{
	unsigned int bauddiv = ((F_CPU+(baudrate*8L))/(baudrate*16L)-1);//
	UBRRL = bauddiv;
	#ifdef UBRRH
	UBRRH = ((bauddiv>>8) & 0x0F);
	// URSEL   7
	// UMSEL   6	0:Asynchronuos/1:Synchronous
	// UPM1    5	Parity mode: 00:disabled/01:Reserved/10:Even/11:Odd
	// UPM0    4	
	// USBS    3	Stop bit: 0:1 bit/1:2 bit
	// UCSZ1   2	Char size:000:5/001:6/010:7/011:8/111:9/others:reserverd
	// UCSZ0   1
	// UCPOL   0
	UCSRC = 0x80 | (1<<UCSZ1) | (1<<UCSZ0);
	#endif
	UCR =((1 << TXEN) | (1 << RXEN) | (1<< RXCIE) | (1<< TXCIE));// 
	UartTxBufferStart = 0;
	UartTxBufferLen = 0;
	UartRxBufferStart = 0;
	UartRxBufferLen = 0;
	sei();
}
//--------------------------------------------------------------------------------------
SIGNAL(SIG_UART_TRANS)
{
	if(UartTxBufferLen){
		--UartTxBufferLen;
		UDR = UartTxBuffer[UartTxBufferStart++];
		if (UartTxBufferStart == UART_TX_BUFFER_SIZE)
   			UartTxBufferStart = 0;
	}
}
//--------------------------------------------------------------------------------------
SIGNAL(SIG_UART_RECV)
{
	unsigned char i;
	char status,data;
	status = USR;
	data = UDR;
	if ((status & ((1<<FE) | (1<<PE) | (1<<DOR))) == 0){
		if(++UartRxBufferLen == UART_RX_BUFFER_SIZE)
			UartRxBufferLen = UART_RX_BUFFER_SIZE;
		i = UartRxBufferStart+UartRxBufferLen;	//Vi tri ky tu cuoi cung trong buffer
		if(i > UART_RX_BUFFER_SIZE)
			i -= UART_RX_BUFFER_SIZE;
		UartRxBuffer[i-1] = data;
	}
}
//--------------------------------------------------------------------------------------
char uartGetByte(void)
{
	//
	char c;
	if(UartRxBufferLen){
		UartRxBufferLen--;
		c = UartRxBuffer[UartRxBufferStart++];
		if(UartRxBufferStart == UART_RX_BUFFER_SIZE)
			UartRxBufferStart = 0;
		return(c);
	}
	return(-1);
}
//--------------------------------------------------------------------------------------
void uartSendByte(char c)
{
	unsigned char i;
	if((USR & (1<<UDRE)) && (UartTxBufferLen == 0)){	//Neu uart dang san sang va buffer trong
		UDR = c;			//Gui luon
	}else{
		//Neu uart dang ban
		while(UartTxBufferLen == UART_TX_BUFFER_SIZE);	//Cho neu buffer dang day
		i = UartTxBufferStart + UartTxBufferLen;
		UartTxBufferLen++;
		if(i >= UART_TX_BUFFER_SIZE)
			i -=UART_TX_BUFFER_SIZE;
		UartTxBuffer[i] = c;	//Ghi vao cuoi buffer
	}
}
//--------------------------------------------------------------------------------------
int printfP(const prog_char *format, ...)
{
	// simple printf routine
	// define a global HexChars or use line below
	//static char HexChars[16] = "0123456789ABCDEF";
	char c;
	unsigned int u_val, div_val, base;
	va_list ap;
	va_start(ap, format);
	for (;;)
	{
		while ((c = pgm_read_byte(format++) ) != '%')
		{	// Until '%' or '\0'
			if (!c)
			{
				va_end(ap);
				return(0);
			}
			uartSendByte(c);
		}

		switch (c = pgm_read_byte(format++) )
		{
			case 'c': c = va_arg(ap,int);
			default:  uartSendByte(c); continue;
			case 'd': base = 10; div_val = 10000; goto CONVERSION_LOOP;
//			case 'x': base = 16; div_val = 0x10;
			case 'x': base = 16; div_val = 0x1000;

			CONVERSION_LOOP:
			u_val = va_arg(ap,int);
			if (c == 'd')
			{
				if (((int)u_val) < 0)
				{
					u_val = - u_val;
					uartSendByte('-');
				}
				while (div_val > 1 && div_val > u_val) div_val /= 10;
			}
			do
			{
				//c =pgm_read_byte(HexTable+(u_val/div_val));
				uartSendByte(pgm_read_byte(HexTable+(u_val/div_val)));
				u_val %= div_val;
				div_val /= base;
			} while (div_val);
		}
	}
	va_end(ap);
}
//--------------------------------------------------------------------------------------
