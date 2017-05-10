//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef IP_H
#define IP_H
//----------------------------------------------------------------------------
#include "packet.h"

struct ipConfig				///< IP addressing/configuration structure
{
	unsigned long ip;			///< IP address
	unsigned long netmask;		///< netmask
	unsigned long gateway;		///< gateway IP address
	struct ntEthAddr ethaddr;
};

#define IP_TIME_TO_LIVE		128		///< default Time-To-Live (TTL) value to use in IP headers
//----------------------------------------------------------------------------
unsigned int ipChecksum(unsigned char *data, unsigned int len);
void ipSetConfig(unsigned long myIp, unsigned long netmask, unsigned long gatewayIp);
struct ipConfig* ipGetConfig(void);
void ethPrintAddr(struct ntEthAddr* ethAddr);
void ipPrintAddr(unsigned long ipaddr);
void ipPrintConfig(struct ipConfig* config);
void ipSend(unsigned long dstIp, unsigned char protocol, unsigned int len, unsigned char* data);
void IPProcess(unsigned int len, struct ntIPHeader* packet);
//----------------------------------------------------------------------------

#endif //IP_H
