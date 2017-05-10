//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include "packet.h"
#include "ip.h"
#include "uart.h"
#include "dhcp.h"
//#define UDP_DEBUG
//----------------------------------------------------------------------------
//Ham gui di mot goi UDP
void udpSend(unsigned long dstIp, unsigned int dstPort, unsigned int srcPort, unsigned int len, unsigned char* udpData)
{
	struct ntUDPHeader* udpHeader;
	udpHeader = (struct ntUDPHeader*)(udpData - UDP_HEADER_LEN);
	len += UDP_HEADER_LEN;
	udpHeader->desPort = HTONS(dstPort);
	udpHeader->srcPort  = HTONS(srcPort);
	udpHeader->Len = HTONS(len);
	udpHeader->Checksum = 0;
	ipSend(dstIp, IP_PROTO_UDP, len, (unsigned char*)udpHeader);
}
//--------------------------------------------------------------------------------------
//Ham xu ly goi UDP nhan duoc, duoc goi boi ham xu ly goi IP (IPProcess)
// Hien chua co ung dung chay UDP nen ham nay trong
void UDPProcess(unsigned int len, struct ntIPHeader* packet)
{
	dhcpIn((len - IP_HEADER_LEN - UDP_HEADER_LEN), (struct netDhcpHeader*)((char*)packet + IP_HEADER_LEN + UDP_HEADER_LEN));
	#ifdef UDP_DEBUG
	printf("Rx UDP Packet\r\n");
	#endif
}
//--------------------------------------------------------------------------------------

