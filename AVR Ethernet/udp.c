/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include "packet.h"
#include "ip.h"
#include "uart.h"
#include "dhcp.h"
//New
#include <avr/io.h>
#include "ethernet.h"
#include "arp.h"
#include "tcp.h"////
#include "udp.h"////
#include "icmp.h"
extern struct ipConfig IpMyConfig;	///< Local IP address/config structure
//#define UDP_DEBUG
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
	
	//ipSend(dstIp, IP_PROTO_UDP, len, (unsigned char*)udpHeader);
	//void ipSend(unsigned long dstIp, unsigned char protocol, unsigned int len, unsigned char* ipData)

	struct ntEthHeader* ethHeader;
	struct ntIPHeader* ipHeader;
	ipHeader = (struct ntIPHeader*)((unsigned char*)udpHeader - IP_HEADER_LEN);
	ethHeader = (struct ntEthHeader*)((unsigned char*)udpHeader - IP_HEADER_LEN - ETH_HEADER_LEN);
	len += IP_HEADER_LEN;
	ipHeader->desIPAddr = HTONL(dstIp);
	ipHeader->srcIPAddr = HTONL(IpMyConfig.ip);
	ipHeader->Protocol = IP_PROTO_UDP;
	ipHeader->Len = HTONS(len);
	ipHeader->verHdrLen = 0x45;
	ipHeader->ToS = 0;
	ipHeader->IDNumber = 0;
	ipHeader->Offset = 0;
	ipHeader->TTL = IP_TIME_TO_LIVE;
	ipHeader->Checksum = 0;

	ipHeader->Checksum = ipChecksum((unsigned char*)ipHeader, IP_HEADER_LEN);
	if( (dstIp & IpMyConfig.netmask) == (IpMyConfig.ip & IpMyConfig.netmask) )
	{
		arpIpOut((unsigned char*)ethHeader,0);					// local send
	}
	else
	{
		arpIpOut((unsigned char*)ethHeader,IpMyConfig.gateway);	// gateway send
	}
	len += ETH_HEADER_LEN;
	ethSendFrame(len, (unsigned char*)ethHeader);
}
//--------------------------------------------------------------------------------------
//Ham xu ly goi UDP nhan duoc, duoc goi boi ham xu ly goi IP (IPProcess)
// Hien chua co ung dung chay UDP nen ham nay trong
void UDPProcess(unsigned int len, struct ntIPHeader* packet)
{
	dhcpIn((len - IP_HEADER_LEN - UDP_HEADER_LEN), (struct netDhcpHeader*)((char*)packet + IP_HEADER_LEN + UDP_HEADER_LEN));
}
//--------------------------------------------------------------------------------------

