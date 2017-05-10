//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include <avr/io.h>
#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
#include "tcp.h"////
#include "udp.h"////
#include "icmp.h"
#include "uart.h"
//--------------------------------------------------------------------------------------
struct ipConfig IpMyConfig;	///< Local IP address/config structure
//--------------------------------------------------------------------------------------
//Ham tinh checksum cho goi ip
unsigned int ipChecksum(unsigned char *data, unsigned int len)
{
    register unsigned long sum = 0;

    for (;;) {
        if (len < 2)
            break;
		sum += *((unsigned int *)data);
		data+=2;
        len -= 2;
    }
    if (len)
        sum += *(unsigned char *) data;

    while ((len = (unsigned int) (sum >> 16)) != 0)
        sum = (unsigned int) sum + len;

    return (unsigned int) sum ^ 0xFFFF;
}
//--------------------------------------------------------------------------------------
//Set cac gia tri cau hinh cho giao thuc ip
void ipSetConfig(unsigned long myIp, unsigned long netmask, unsigned long gatewayIp)
{
	/*
	// set local addressing
	IpMyConfig.ip = myIp;
	IpMyConfig.netmask = netmask;
	IpMyConfig.gateway = gatewayIp;
	ethGetMacAddress(IpMyConfig.ethaddr.addr);
	*/
	struct ntEthAddr ethaddr;

	// set local addressing
	IpMyConfig.ip = myIp;
	IpMyConfig.netmask = netmask;
	IpMyConfig.gateway = gatewayIp;

	// set ARP association
	ethGetMacAddress(ethaddr.addr);
	arpSetAddress(&ethaddr, myIp);

}
//--------------------------------------------------------------------------------------
//Tra lai con tro den struct ipConfig
struct ipConfig* ipGetConfig(void)
{
	return &IpMyConfig;
}
//--------------------------------------------------------------------------------------
//In ra dia chi ethernet
void ethPrintAddr(struct ntEthAddr* ethAddr)
{
	printf("%x:%x:%x:%x:%x:%x",(ethAddr->addr[0]),(ethAddr->addr[1]),(ethAddr->addr[2]),\
		(ethAddr->addr[3]),(ethAddr->addr[4]),(ethAddr->addr[5]));
}
//--------------------------------------------------------------------------------------
//In ra dia chi IP
void ipPrintAddr(unsigned long ipaddr)
{
	printf("%d.%d.%d.%d",
		((unsigned char*)&ipaddr)[3],
		((unsigned char*)&ipaddr)[2],
		((unsigned char*)&ipaddr)[1],
		((unsigned char*)&ipaddr)[0]);
}
//--------------------------------------------------------------------------------------
//In ra cac gia tri cau hinh cho giao thuc IP
void ipPrintConfig(struct ipConfig* config)
{
	printf("IP Addr : "); ipPrintAddr(config->ip);		printf("\n\r");
	printf("Netmask : "); ipPrintAddr(config->netmask);	printf("\n\r");
	printf("Gateway : "); ipPrintAddr(config->gateway);	printf("\n\r");
}
//--------------------------------------------------------------------------------------
//Ham gui 1 goi IP
void ipSend(unsigned long dstIp, unsigned char protocol, unsigned int len, unsigned char* ipData)
{
	struct ntEthHeader* ethHeader;
	struct ntIPHeader* ipHeader;
	ipHeader = (struct ntIPHeader*)(ipData - IP_HEADER_LEN);
	ethHeader = (struct ntEthHeader*)(ipData - IP_HEADER_LEN - ETH_HEADER_LEN);
	len += IP_HEADER_LEN;

	ipHeader->desIPAddr = HTONL(dstIp);
	ipHeader->srcIPAddr = HTONL(IpMyConfig.ip);
	ipHeader->Protocol = protocol;
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
	//#ifdef IP_DEBUG
	printf("Sending IP packet\r\nAddr: ");
	ipPrintAddr(dstIp);printf("\n\rMAC: ");
	ethPrintAddr(&(ethHeader->desAddr));printf("\n\r");
	//#endif
	ethSendFrame(len, (unsigned char*)ethHeader);
}
//--------------------------------------------------------------------------------------
//Ham x? ly goi IP, duoc goi boi giao thuc ethernet khi paket type duoc xac dinh la IP
void IPProcess(unsigned int len, struct ntIPHeader* packet)
{
	// check IP addressing, stop processing if not for me and not a broadcast
	if( (HTONL(packet->desIPAddr) != ipGetConfig()->ip) &&
		(HTONL(packet->desIPAddr) != (ipGetConfig()->ip|ipGetConfig()->netmask)) &&
		(HTONL(packet->desIPAddr) != 0xFFFFFFFF) &&
		(ipGetConfig()->ip != 0x00000000) ) 
		return;

	// handle ICMP packet
	if( packet->Protocol == IP_PROTO_ICMP )
	{
		#ifdef IP_DEBUG
		printf("IP->Rx: ICMP/IP packet\r\n");
		//icmpPrintHeader((icmpip_hdr*)packet);
		#endif
		icmpIpIn((struct ntIPHeader*)packet);
	}
	else if( packet->Protocol == IP_PROTO_UDP )
	{
		#ifdef IP_DEBUG
		printf("IP->Rx: UDP/IP packet\r\n");
		//debugPrintHexTable(NetBufferLen-14, &NetBuffer[14]);
		#endif
		UDPProcess(len, ((struct ntIPHeader*)packet) );////
	}
	else if( packet->Protocol == IP_PROTO_TCP )
	{
		#ifdef IP_DEBUG
		printf("IP->Rx: TCP/IP packet\r\n");
		#endif
		TCPProcess((unsigned char *)packet,len-((packet->verHdrLen & 0x0F)<<2));////
	}
	else
	{
		#ifdef IP_DEBUG
		printf("IP->Rx: IP packet\r\n");
		#endif
	}
}
//--------------------------------------------------------------------------------------
