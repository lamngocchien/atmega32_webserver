/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
#include "icmp.h"
//Ham gui di ban tin tra loi cho Echo Request (Echo Reply)
void icmpEchoReply(struct ntIPHeader* ipHeader)
{
	unsigned long tempIp;
	unsigned char* ethFrame;
	struct ntICMPHeader* icmpHeader;
	icmpHeader = (struct ntICMPHeader*)((unsigned char*)ipHeader + IP_HEADER_LEN);
	icmpHeader->Type = ICMP_TYPE_ECHOREPLY;
	icmpHeader->Checksum = 0;
	icmpHeader->Checksum = ipChecksum((unsigned char*)icmpHeader, HTONS(ipHeader->Len)-IP_HEADER_LEN);
	tempIp = ipHeader->desIPAddr;
	ipHeader->desIPAddr = ipHeader->srcIPAddr;
	ipHeader->srcIPAddr = tempIp;
	ethFrame = ((unsigned char*)ipHeader);
	ethFrame -= ETH_HEADER_LEN;
	arpIpOut(ethFrame, 0);
	ethSendFrame(HTONS(ipHeader->Len)+ETH_HEADER_LEN, ethFrame);
}
//Ham xu ly goi ICMP nhan duoc
void icmpIpIn(struct ntIPHeader* ipHeader)
{
	struct ntICMPHeader* icmpHeader;
	icmpHeader = (struct ntICMPHeader*)((unsigned char*)ipHeader + IP_HEADER_LEN);
	// check ICMP type
	switch(icmpHeader->Type)
	{
	case ICMP_TYPE_ECHOREQUEST:
		// echo request
		icmpEchoReply(ipHeader);
		break;
	default:
		break;
	}
}
