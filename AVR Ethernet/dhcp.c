/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include "packet.h"
#include "ethernet.h"
#include "ip.h"
#include "udp.h"
#include "dhcp.h"
#include "uart.h"
#include "AVRnet.h"
#include <avr/pgmspace.h>

unsigned long DhcpServerIP;
unsigned long DhcpTransactID;
unsigned long DhcpLeaseTime;
unsigned char macaddr[6];

unsigned char DhcpTimeout;
unsigned char DhcpRetries;

//Ham khoi tao cac thong so ban dau cho DHCP
void dhcpInit(void)
{
	ethGetMacAddress(macaddr);
	DhcpTransactID = *((unsigned long*)&macaddr[0]);
	DhcpLeaseTime = 0;
	DhcpTimeout = 1;
	DhcpRetries = DHCP_RETRIES;
}

//Ham set cac option cua DHCP
unsigned char* dhcpSetOption(unsigned char* options, unsigned char optcode, unsigned char optlen, void* optvalptr)
{
	*options++ = optcode;
	*options++ = optlen;
	while(optlen--)
	{
		*options++ = *(unsigned char*)optvalptr++;
	}
	*options = DHCP_OPT_END;
	return options;
}

//Ham lay cac option cua DHCP
unsigned char dhcpGetOption(unsigned char* options, unsigned char optcode, unsigned char optlen, void* optvalptr)
{
	unsigned char i;
	for (;;)
	{
		if(*options == DHCP_OPT_PAD)
			options++;
		else if(*options == DHCP_OPT_END)
			break;
		else if(*options == optcode)
		{
			optlen = ((optlen<*(options+1))?(optlen):(*(options+1)));
			for(i=0; i<optlen; i++)
				*(((uint8_t*)optvalptr)+i) = *(options+i+2);
			return *(options+1);
		}
		else
		{
			options++;
			options+=*options;
			options++;
		}
	}
	return 0;
}

//Ham gui di mot ban tin DHCP discover de tim kiem DHCP server
void dhcpDiscover(void)
{
	struct netDhcpHeader* packet;
	unsigned long val;
	unsigned char* optptr;
	
	packet = (struct netDhcpHeader*)(ethGetBuffer() + ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN);
	//
	packet->bootp.opcode = BOOTP_OP_BOOTREQUEST;
	packet->bootp.hwaddrtype = BOOTP_HTYPE_ETHERNET;
	packet->bootp.hwaddrlen = BOOTP_HLEN_ETHERNET;
	packet->bootp.clipaddr = HTONL(ipGetConfig()->ip);
	packet->bootp.yoipaddr = HTONL(0l);
	packet->bootp.seipaddr = HTONL(0l);
	packet->bootp.gwipaddr = HTONL(0l);
	ethGetMacAddress(&packet->bootp.clhwaddr[0]);
	packet->bootp.transid = DhcpTransactID;
	packet->bootp.flags = HTONS(1);
	packet->cookie = 0x63538263;
	val = DHCP_MSG_DHCPDISCOVER;
	optptr = dhcpSetOption(packet->options, DHCP_OPT_DHCPMSGTYPE, 1, &val);
	dhcpSetOption(optptr, DHCP_OPT_CLIENTID, 6, macaddr);
	udpSend(0xFFFFFFFF, DHCP_UDP_SERVER_PORT, DHCP_UDP_CLIENT_PORT, DHCP_HEADER_LEN+3+1+8, (unsigned char*)packet);
}

//Ham gui di mot ban tin DHCP request de yeu cau nhan dia chi IP
void dhcpRequest(struct netDhcpHeader* packet, unsigned long serverid)
{
	unsigned char* optptr;
	unsigned long val;

	packet->bootp.opcode = BOOTP_OP_BOOTREQUEST;		// request type
	val = DHCP_MSG_DHCPREQUEST;
	optptr = dhcpSetOption(packet->options, DHCP_OPT_DHCPMSGTYPE, 1, &val);
	optptr = dhcpSetOption(optptr, DHCP_OPT_CLIENTID, 6, macaddr);
	optptr = dhcpSetOption(optptr, DHCP_OPT_SERVERID, 4, &serverid);
	optptr = dhcpSetOption(optptr, DHCP_OPT_REQUESTEDIP, 4, &packet->bootp.yoipaddr);
	((unsigned char*)&val)[0] = DHCP_OPT_NETMASK;
	((unsigned char*)&val)[1] = DHCP_OPT_ROUTERS;
	((unsigned char*)&val)[2] = DHCP_OPT_DNSSERVERS;
	((unsigned char*)&val)[3] = DHCP_OPT_DOMAINNAME;
	optptr = dhcpSetOption(optptr, DHCP_OPT_PARAMREQLIST, 4, &val);
	packet->bootp.yoipaddr = HTONL(0l);
	udpSend(0xFFFFFFFF, DHCP_UDP_SERVER_PORT, DHCP_UDP_CLIENT_PORT, DHCP_HEADER_LEN+3+6+6+6+8+1, (unsigned char*)packet);
}

//Ham xu ly mot goi DHCP nhan duoc
void dhcpIn(unsigned int len, struct netDhcpHeader* packet)
{
	unsigned char msgtype;
	unsigned long sid;
	unsigned long netmask;
	unsigned long gateway;
	unsigned long val;
	if((packet->bootp.opcode != BOOTP_OP_BOOTREPLY) || (packet->bootp.transid != DhcpTransactID))
		{
		//ethInit();
		return;	
		}
	dhcpGetOption(packet->options, DHCP_OPT_DHCPMSGTYPE, 1, &msgtype);
	if(msgtype == DHCP_MSG_DHCPOFFER)
	{
		dhcpGetOption(packet->options, DHCP_OPT_SERVERID, 4, &sid);
		dhcpRequest(packet, sid);
	}
	//
	else if(msgtype == DHCP_MSG_DHCPACK)
	{
		dhcpGetOption(packet->options, DHCP_OPT_NETMASK, 4, &val);
		netmask = HTONL(val);
		dhcpGetOption(packet->options, DHCP_OPT_ROUTERS, 4, &val);
		gateway = HTONL(val);
		dhcpGetOption(packet->options, DHCP_OPT_LEASETIME, 4, &val);
		DhcpLeaseTime = HTONL(val);
		ipSetConfig(HTONL(packet->bootp.yoipaddr), netmask, gateway);
		DhcpRetries = 0;
	}
}
//Ham release dia chi IP hien tai va xoa cac thong so cau hinh IP dang co
void dhcpRelease(void)
{
	struct netDhcpHeader* packet;
	unsigned long val;
	unsigned char* optptr;
	
	packet = (struct netDhcpHeader*)&ethGetBuffer()[ETH_HEADER_LEN+IP_HEADER_LEN+UDP_HEADER_LEN];

	// build BOOTP/DHCP header
	packet->bootp.opcode = BOOTP_OP_BOOTREQUEST;		// request type
	packet->bootp.hwaddrtype = BOOTP_HTYPE_ETHERNET;
	packet->bootp.hwaddrlen = BOOTP_HLEN_ETHERNET;
	packet->bootp.clipaddr = HTONL(ipGetConfig()->ip);
	packet->bootp.yoipaddr = HTONL(0l);
	packet->bootp.seipaddr = HTONL(0l);
	packet->bootp.gwipaddr = HTONL(0l);
	ethGetMacAddress(&packet->bootp.clhwaddr[0]);	// fill client hardware address
	packet->bootp.transid = DhcpTransactID;			// set trans ID (use part of MAC address)
	packet->bootp.flags = HTONS(1);
	
	// build DHCP request
	// begin with magic cookie
	packet->cookie = 0x63538263;
	//
	// set operation
	val = DHCP_MSG_DHCPRELEASE;
	optptr = dhcpSetOption(packet->options, DHCP_OPT_DHCPMSGTYPE, 1, &val);
	// set the server ID
	val = HTONL(DhcpServerIP);
	optptr = dhcpSetOption(optptr, DHCP_OPT_SERVERID, 4, &val);
	// request the IP previously offered
	optptr = dhcpSetOption(optptr, DHCP_OPT_REQUESTEDIP, 4, &packet->bootp.clipaddr);
	// send release
	udpSend(DhcpServerIP, DHCP_UDP_SERVER_PORT, DHCP_UDP_CLIENT_PORT, DHCP_HEADER_LEN+3+6+6+1, (unsigned char*)packet);
	
	// deconfigure ip addressing
	ipSetConfig(0,0,0);
	DhcpLeaseTime = 0;
}

//Ham duoc goi dinh ky moi 1s de cap nhat lease time va timeout cua DHCP
void dhcpTimer(void)
{
	// this function to be called once per second
	// decrement lease time
	if(DhcpLeaseTime)
		DhcpLeaseTime--;
	if(DhcpTimeout){
		DhcpTimeout--;
	}
}
//Ham dich vu DHCP, duoc goi trong chuong trinh chinh
void dhcpService(void)
{
	if(DhcpRetries && (DhcpTimeout == 0)){
		DhcpRetries--;
		DhcpTimeout = DHCP_TIMEOUT;
		dhcpDiscover();
	}
}


