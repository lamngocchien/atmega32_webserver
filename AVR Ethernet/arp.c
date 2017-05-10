/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include <avr/io.h>
#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
struct ARPentry ARPMyAddr;
struct ARPentry ARPTable[ARP_TABLE_SIZE];
void arpInit(void)
{
	unsigned char i;
	for(i=0; i<ARP_TABLE_SIZE; i++)
	{
		ARPTable[i].ipAddr = 0;
		ARPTable[i].time = 0;
	}
}
void arpSetAddress(struct ntEthAddr* ethAddr, unsigned long ipAddr)
{
	ARPMyAddr.ethAddr = *ethAddr;
	ARPMyAddr.ipAddr = ipAddr;
}
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
void arpUpdateEntry(struct ntEthAddr ethAddr,unsigned long ipAddr)
{
	unsigned char index;
	index = arpSearchIP(ipAddr);
	if(index < ARP_TABLE_SIZE)
	{
		ARPTable[index].ethAddr = ethAddr;
		ARPTable[index].time = ARP_CACHE_TIME_TO_LIVE;
		return;
	}
	for(index=0; index<ARP_TABLE_SIZE; index++)
	{
		if(!ARPTable[index].time)
		{
			ARPTable[index].ethAddr = ethAddr;
			ARPTable[index].ipAddr = ipAddr;
			ARPTable[index].time = ARP_CACHE_TIME_TO_LIVE;
			return;
		}
	}
}
void arpArpProcess(unsigned int len, unsigned char* ethFrame)
{
	struct ntEthHeader* ethHeader;
	struct ntARPHeader* arpHeader;
	ethHeader = (struct ntEthHeader*)ethFrame;
	arpHeader = (struct ntARPHeader*)(ethFrame + ETH_HEADER_LEN);
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
			ethSendFrame(len, (unsigned char*)ethHeader);
			return;
		}
		if(arpHeader->opcode == HTONS(ARP_OPCODE_REPLY)){
			arpUpdateEntry(arpHeader->shwaddr,HTONL(arpHeader->sipaddr));
			return;
		}
	}
}
void arpIPPacketIn(unsigned char* ethFrame)
{
	struct ntEthHeader* ethHeader;
	struct ntIPHeader* ipHeader;
	ethHeader = (struct ntEthHeader*)ethFrame;
	ipHeader = (struct ntIPHeader*)(ethFrame + ETH_HEADER_LEN);
	arpUpdateEntry(ethHeader->srcAddr,HTONL(ipHeader->srcIPAddr));
}
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
}
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