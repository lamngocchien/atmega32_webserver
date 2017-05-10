//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include <avr/io.h>
#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
//--------------------------------------------------------------------------------------
struct ARPentry ARPMyAddr;
struct ARPentry ARPTable[ARP_TABLE_SIZE];
//--------------------------------------------------------------------------------------
void arpInit(void)
{
	unsigned char i;
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		ARPTable[i].ipAddr = 0;
		ARPTable[i].time = 0;
	}
}
//--------------------------------------------------------------------------------------
void arpSetAddress(struct ntEthAddr* ethAddr, unsigned long ipAddr)
{
	ARPMyAddr.ethAddr = *ethAddr;
	ARPMyAddr.ipAddr = ipAddr;
}
//--------------------------------------------------------------------------------------
unsigned char arpSearchIP(unsigned long ipaddr)
{
	unsigned char i;
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		if((ARPTable[i].ipAddr == ipaddr) && (ARPTable[i].time != 0))
		{
			return i;
		}
	}
	return -1;
}
//--------------------------------------------------------------------------------------
void arpUpdateEntry(struct ntEthAddr ethAddr,unsigned long ipAddr)
{
	unsigned char index;
	index = arpSearchIP(ipAddr);
	if(index < ARP_TABLE_SIZE)
	{
		ARPTable[index].ethAddr = ethAddr;
		ARPTable[index].time = ARP_CACHE_TIME_TO_LIVE;
		#ifdef ARP_DEBUG
		printf("Update ARP TTL %d: ",index);ipPrintAddr(ipAddr);
		printf("-");ethPrintAddr(&ethAddr);printf("\n\r");
		#endif
		return;
	}
	for(index=0; index<ARP_TABLE_SIZE; index++)
	{
		if(!ARPTable[index].time)
		{
			ARPTable[index].ethAddr = ethAddr;
			ARPTable[index].ipAddr = ipAddr;
			ARPTable[index].time = ARP_CACHE_TIME_TO_LIVE;
			#ifdef ARP_DEBUG
			printf("Update ARP: ");ipPrintAddr(ipAddr);
			printf("-");ethPrintAddr(&ethAddr);printf("\n\r");
			#endif
			return;
		}
	}
}
//--------------------------------------------------------------------------------------
void arpArpProcess(unsigned int len, unsigned char* ethFrame)
{
	struct ntEthHeader* ethHeader;
	struct ntARPHeader* arpHeader;
	ethHeader = (struct ntEthHeader*)ethFrame;
	arpHeader = (struct ntARPHeader*)(ethFrame + ETH_HEADER_LEN);
	#ifdef ARP_DEBUG
	printf("Received ARP Request\r\n");
	arpPrintHeader(arpHeader);
	#endif
	if(	(arpHeader->hwType == 0x0100) &&
		(arpHeader->protocol == 0x0008)  &&
		(arpHeader->hwLen == 0x06) && 
		(arpHeader->protoLen == 0x04) &&
		(arpHeader->dipaddr == HTONL(ARPMyAddr.ipAddr))){
		if(arpHeader->opcode == HTONS(ARP_OPCODE_REQUEST)){
			arpUpdateEntry(arpHeader->shwaddr,HTONL(arpHeader->sipaddr));
			arpHeader->dhwaddr = arpHeader->shwaddr;
			arpHeader->dipaddr = arpHeader->sipaddr;
			arpHeader->shwaddr = ARPMyAddr.ethAddr;
			arpHeader->sipaddr = HTONL(ARPMyAddr.ipAddr);
			arpHeader->opcode = HTONS(ARP_OPCODE_REPLY);
			ethHeader->desAddr = ethHeader->srcAddr;
			ethHeader->srcAddr  = ARPMyAddr.ethAddr;
			#ifdef ARP_DEBUG
			printf("Sending ARP Reply\r\n");
			arpPrintHeader(arpHeader);
			#endif
			ethSendFrame(len, (unsigned char*)ethHeader);
			return;
		}
		if(arpHeader->opcode == HTONS(ARP_OPCODE_REPLY)){
			arpUpdateEntry(arpHeader->shwaddr,HTONL(arpHeader->sipaddr));
			#ifdef ARP_DEBUG
			printf("is ARP reply\r\n");
			#endif
			return;
		}
	}
	#ifdef ARP_DEBUG
	printf("Unknown ARP packet\r\n");
	#endif
}
//--------------------------------------------------------------------------------------
void arpIPPacketIn(unsigned char* ethFrame)
{
	struct ntEthHeader* ethHeader;
	struct ntIPHeader* ipHeader;
	ethHeader = (struct ntEthHeader*)ethFrame;
	ipHeader = (struct ntIPHeader*)(ethFrame + ETH_HEADER_LEN);
	arpUpdateEntry(ethHeader->srcAddr,HTONL(ipHeader->srcIPAddr));
}
//--------------------------------------------------------------------------------------
void arpIpOut(unsigned char* ethFrame, unsigned long phyDstIp)
{
	unsigned char index;
	struct ntEthHeader* ethHeader;
	struct ntIPHeader* ipHeader;
	ethHeader = (struct ntEthHeader*)ethFrame;
	ipHeader = (struct ntIPHeader*)(ethFrame + ETH_HEADER_LEN);

	if(phyDstIp)
		index = arpSearchIP(phyDstIp);
	else
		index = arpSearchIP(HTONL(ipHeader->desIPAddr));
	if(index < ARP_TABLE_SIZE)
	{
		ethHeader->srcAddr  = ARPMyAddr.ethAddr;
		ethHeader->desAddr = ARPTable[index].ethAddr;
		ethHeader->type = HTONS(ETH_TYPE_IP);
	}
	else
	{
		ethHeader->srcAddr = ARPMyAddr.ethAddr;
		ethHeader->desAddr.addr[0] = 0xFF;
		ethHeader->desAddr.addr[1] = 0xFF;
		ethHeader->desAddr.addr[2] = 0xFF;
		ethHeader->desAddr.addr[3] = 0xFF;
		ethHeader->desAddr.addr[4] = 0xFF;
		ethHeader->desAddr.addr[5] = 0xFF;
		ethHeader->type = HTONS(ETH_TYPE_IP);
	}
	#ifdef ARP_DEBUG
	printf("ARP Result:");
	ipPrintAddr(ARPTable[index].ipAddr);printf("-");
	ethPrintAddr(&(ethHeader->desAddr));printf("\r\n");
	#endif
}
//--------------------------------------------------------------------------------------
void arpTimer(void)
//Goi moi 10s
{
	int index;
	for(index=0; index<ARP_TABLE_SIZE; index++)
	{
		if(ARPTable[index].time)
			ARPTable[index].time--;
	}
}
//--------------------------------------------------------------------------------------
#ifdef ARP_DEBUG
void arpPrintHeader(struct ntARPHeader* packet)
{
	printf("ARP Packet:\r\n");
	printf("Operation   : ");
	if(packet->opcode == HTONS(ARP_OPCODE_REQUEST))
		printf("REQUEST");
	else if(packet->opcode == HTONS(ARP_OPCODE_REPLY))
		printf("REPLY");
	else
		printf("UNKNOWN");
	printf("\n\r");
	printf("SrcHwAddr   : ");	ethPrintAddr(&packet->shwaddr);printf("\n\r");
	printf("SrcProtoAddr: ");	ipPrintAddr(HTONL(packet->sipaddr));printf("\n\r");
	printf("DstHwAddr   : ");	ethPrintAddr(&packet->dhwaddr);printf("\n\r");
	printf("DstProtoAddr: ");	ipPrintAddr(HTONL(packet->dipaddr));printf("\n\r");
}
//--------------------------------------------------------------------------------------
void arpPrintTable(void)
{
	unsigned char i;

	// print ARP table
	printf("Time    Eth Address    IP Address\r\n");
	printf("---------------------------------------\r\n");
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		printf("%d",(ARPTable[i].time));
		printf("   ");
		ethPrintAddr(&ARPTable[i].ethAddr);
		printf("  ");
		ipPrintAddr(ARPTable[i].ipAddr);
		printf("\n\r");
	}
}
#endif
//--------------------------------------------------------------------------------------
