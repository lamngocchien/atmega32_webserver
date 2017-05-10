/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef TCP_H
#define TCP_H
#include <avr/pgmspace.h>////
//
#define MAX_SEGMENT_SIZE	(ETHERNET_BUFFER_SIZE - ETH_HEADER_LEN - IP_HEADER_LEN - TCP_HEADER_LEN)
//
//List the TCP session state
#define TCP_STATE_CLOSED       0
#define TCP_STATE_SYN_SENT     1
#define TCP_STATE_LISTEN       2
#define TCP_STATE_SYN_RECEIVED 3
#define TCP_STATE_ESTABLISHED  4
#define TCP_STATE_FIN_WAIT1    5
#define TCP_STATE_FIN_WAIT2    6
#define TCP_STATE_CLOSING      7
#define TCP_STATE_TIMED_WAIT   8
#define TCP_STATE_CLOSE_WAIT   9
#define TCP_STATE_LAST_ACK    10
//60 seconds timeout:
#define TCP_TIMEOUT 60
//maximum connection count
#define TCP_MAX_SESSION 8
struct tcpSession{
	unsigned int  desPort;		//Port on the remote host
	unsigned int  srcPort;		//Port on the local host
	unsigned long desIP;		//IP address of the remote host
	unsigned long seqNumber;	//Sequence number
	unsigned long ackNumber;	//Acknowlegement number
	unsigned char sesState;	//Current state of TCP session
	unsigned int  srcWin;		
	unsigned int  desWin;		
	unsigned long lastRxAck;	//Last Received Ack
	unsigned char nextAck;
	unsigned char timeOut;	//Session time out
	void(*appDataIn)(unsigned char* dataBuffer,unsigned int dataLen,struct tcpSession *pSession);
	unsigned char appID; 		//Upper layer application ID 
	unsigned char appState; 	//Upper layer application state
};
//
unsigned long TCPInitSequenceNumber();
void TCPCloseSession(unsigned char socketnum);
void TCPInit();
unsigned char TCPGetFreeSession();
void	TCPCreateSession(unsigned int  sourcePort, prog_void* appService);
void TCPCheckTimeOut();
unsigned int checksum(unsigned char *buffer, unsigned int len, unsigned long csum32);
void TCPPackedSend(struct tcpSession *pSession, unsigned char Flags, unsigned int len, unsigned char *dataBuffer);
void TCPProcess(unsigned char *buffer, unsigned int len);
//
#endif //TCP_H
