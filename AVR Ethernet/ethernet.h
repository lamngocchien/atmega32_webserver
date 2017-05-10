/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef ETHERNET_H
#define ETHERNET_H
#ifndef		MTU_SIZE
	#define		MTU_SIZE	1000//1460
#endif
#ifndef		IP_HEADER_LEN
	#define IP_HEADER_LEN	20
#endif	//IP_HEADER_LEN
#ifndef		TCP_HEADER_LEN
#define TCP_HEADER_LEN	20
#endif	//TCP_HEADER_LEN
//
#ifndef ETHERNET_BUFFER_SIZE
#define ETHERNET_BUFFER_SIZE		(MTU_SIZE+ETH_HEADER_LEN+IP_HEADER_LEN+TCP_HEADER_LEN)
#endif
void ethInit(void);
void ethSendFrame(unsigned int len, unsigned char* packet);
unsigned int ethGetFrame(unsigned int maxlen, unsigned char* packet);
void ethGetMacAddress(unsigned char* macaddr);
void ethSetMacAddress(unsigned char* macaddr);
unsigned char* ethGetBuffer(void);
void ethService(void);

#endif //ETHERNET_H
