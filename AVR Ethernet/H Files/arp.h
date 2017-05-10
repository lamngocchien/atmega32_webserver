//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef ARP_H
#define ARP_H
//----------------------------------------------------------------------------
#include "packet.h"
//--------------------------------------------------------------------------------------
#ifndef ARP_TABLE_SIZE
#define ARP_TABLE_SIZE	8
#endif

#ifndef ARP_CACHE_TIME_TO_LIVE
#define ARP_CACHE_TIME_TO_LIVE	250
#endif

//#define ARP_DEBUG_PRINT
//--------------------------------------------------------------------------------------
struct ARPentry
{
	unsigned long ipAddr;			///< remote-note IP address
	struct ntEthAddr ethAddr;	///< remote-node ethernet (hardware/mac) address
	unsigned char time;				///< time to live (in ARP table); this is decremented by arpTimer()
};
//--------------------------------------------------------------------------------------
void arpInit(void);
void arpSetAddress(struct ntEthAddr* myeth, unsigned long myip);
void arpUpdateEntry(struct ntEthAddr ethAddr,unsigned long ipAddr);
void arpArpProcess(unsigned int len, unsigned char* ethFrame);
void arpIPPacketIn(unsigned char* ethFrame);
void arpIpOut(unsigned char* ethFrame, unsigned long phyDstIp);
void arpTimer(void);
unsigned char arpSearchIP(unsigned long ipaddr);
void arpPrintHeader(struct ntARPHeader* packet);
void arpPrintTable(void);
//--------------------------------------------------------------------------------------

#endif //ARP_H
