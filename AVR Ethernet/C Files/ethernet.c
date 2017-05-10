//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "packet.h"
#include "enc28j60.h"
#include "enc28j60conf.h"
#include "ethernet.h"

#include "arp.h"
#include "ip.h"
//----------------------------------------------------------------------------
unsigned char ethBuffer[ETHERNET_BUFFER_SIZE];
unsigned char eth_got_frame = 0;
volatile unsigned int time_watchdog;

//----------------------------------------------------------------------------
//Ham khoi tao chip Ethernet
void ethInit(void)
{
	enc28j60Init();
	ETH_INT_ENABLE;
}
//--------------------------------------------------------------------------------------
//Ham goi 1 frame xuong chip ethernet
void ethSendFrame(unsigned int len, unsigned char* packet)
{
	enc28j60PacketSend(len, packet);
}
//--------------------------------------------------------------------------------------
//Ham doc 1 frame ethernet tu chip ethernet ve buffer tren RAM cua CPU
unsigned int ethGetFrame(unsigned int maxlen, unsigned char* packet)
{
	return enc28j60PacketReceive(maxlen, packet);
}
//--------------------------------------------------------------------------------------
//Ham doc dia chi MAC hien tai tu chip ethernet, luu vao buffer macaddr[6]
void ethGetMacAddress(unsigned char* macaddr)
{
	*macaddr++ = enc28j60Read(MAADR5);
	*macaddr++ = enc28j60Read(MAADR4);
	*macaddr++ = enc28j60Read(MAADR3);
	*macaddr++ = enc28j60Read(MAADR2);
	*macaddr++ = enc28j60Read(MAADR1);
	*macaddr++ = enc28j60Read(MAADR0);
}
//--------------------------------------------------------------------------------------
//Ham set dia chi MAC (dang luu trong buffer macaddr[6] xuong chip ethernet
void ethSetMacAddress(unsigned char* macaddr)
{
	enc28j60Write(MAADR5, *macaddr++);
	enc28j60Write(MAADR4, *macaddr++);
	enc28j60Write(MAADR3, *macaddr++);
	enc28j60Write(MAADR2, *macaddr++);
	enc28j60Write(MAADR1, *macaddr++);
	enc28j60Write(MAADR0, *macaddr++);
}
//--------------------------------------------------------------------------------------
//Ham tra lai con tro den buffer ethernet (tren RAM cua CPU)
unsigned char* ethGetBuffer(void)
{
	return ethBuffer;
}
//--------------------------------------------------------------------------------------
//Vector ngat cua ethernet, mot ngat ngoai se duoc khoi tao boi chip ethernet
// moi khi no nhan duoc 1 frame ethernet (dung dia chi cua no)
ISR (ETH_INTERRUPT)
{
	eth_got_frame = 1;
    time_watchdog = 0;
	ETH_INT_DISABLE;
}
//--------------------------------------------------------------------------------------
//Ham duoc goi lien tuc de thuc thi cac tac vu cua giao thuc ethernet
void ethService(void)
{
	int len;
	struct ntEthHeader* ethPacket;
	if(!eth_got_frame) return;
	// look for a packet
	len = ethGetFrame(ETHERNET_BUFFER_SIZE, ethBuffer);

	if(len)
	{
		ethPacket = (struct ntEthHeader*)&ethBuffer[0];

		#ifdef ETH_DEBUG
		printf("Received packet len: %d, type:", len);
		#endif
		
		if(ethPacket->type == HTONS(ETH_TYPE_IP))
		//Neu day la frame danh cho giao thuc IP
		{
			#ifdef ETH_DEBUG
			printf("IP packet\r\n");
			#endif
			arpIPPacketIn((unsigned char*)&ethBuffer[0]);
			IPProcess( len-ETH_HEADER_LEN, (struct ntIPHeader*)&ethBuffer[ETH_HEADER_LEN] );
		}
		else if(ethPacket->type == HTONS(ETH_TYPE_ARP))
		//Neu day la 1 frame cua giao thuc ARP
		{
			#ifdef ETH_DEBUG
			printf("ARP packet\r\n");
			#endif
			arpArpProcess(len, ethBuffer );
		}else{
			#ifdef ETH_DEBUG
			printf("Unknown packet:%x\r\n",ethPacket->type);
			#endif
			ethInit();
		}
		ETH_INT_ENABLE;
	}
	return;
}
//--------------------------------------------------------------------------------------
