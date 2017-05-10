/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
//#ifndef UART_H
//#define UART_H
//
//#include <avr/pgmspace.h>
//#define UART_TX_BUFFER_SIZE		8
//#define UART_RX_BUFFER_SIZE		8
//#ifndef UART_INTERRUPT_HANDLER
//#define UART_INTERRUPT_HANDLER	SIGNAL
//#endif
////define for ATmega32 register
//#define USR UCSRA
//#define UCR UCSRB
//#define UBRR UBRRL
//#define EICR EICRB
//#define USART_RX USART_RXC_vect  
//#define USART_TX USART_TXC_vect  
//void uartInit(unsigned long baudrate);
//char uartGetByte();
//void uartSendByte(char c);
//int printfP(const prog_char *format, ...);
//#define printf(format, args...) printfP(PSTR(format), ## args)
//#endif //UART_H
//