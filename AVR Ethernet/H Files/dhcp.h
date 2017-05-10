//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef DHCP_H
#define DHCP_H

#include "packet.h"
#define DHCP_DEBUG
//----------------------------------------------------------------------------
/// BOOTP Header
struct ntBootpHeader
{
	unsigned char	opcode;			//Message op-code / message type
	unsigned char	hwaddrtype;		//Hardware address type   (Ethernet=1)
	unsigned char	hwaddrlen;		//Hardware address length (Ethernet=6 byte MAC addr)
	unsigned char	hops;			//hop count (client set to zero)
	unsigned long	transid;		//Transaction ID (randomly chosen by client, must remain same)
	unsigned int	secs;			//Seconds elapsed since DHCP negotiation began (filled by client)
	unsigned int	flags;			//Flags
	unsigned long	clipaddr;		//Client IP address (filled only if already bound, renewing, or rebinding)
	unsigned long	yoipaddr;		//Your IP address (client)
	unsigned long	seipaddr;		//Server IP address
	unsigned long	gwipaddr;		//Gateway IP address
	unsigned char	clhwaddr[16];	//Client Hardware Address
	unsigned char	sename[64];		//Server Host Name
	unsigned char	file[128];		//Boot file name (null-term string)
};
//
#define BOOTP_HEADER_LEN		236	//length of BOOTP header not including options
//
#define BOOTP_OP_BOOTREQUEST	1	//BOOTP Request operation (message from client to server)
#define BOOTP_OP_BOOTREPLY		2	//BOOTP Reply operation (message from server to client)
//
#define BOOTP_HTYPE_ETHERNET	1	//Hardware type for ethernet protocol
#define BOOTP_HLEN_ETHERNET		6	//Length of ethernet MAC address
//----------------------------------------------------------------------------
struct netDhcpHeader
{
	struct ntBootpHeader bootp;		//BOOTP header
	unsigned long	cookie;			//magic cookie value
	unsigned char	options[1];		//DHCP options
};
//
#define DHCP_HEADER_LEN			240	//length of DHCP header not including options
//Code for DHCP option field
#define DHCP_OPT_PAD			0	//token padding value (make be skipped)
#define DHCP_OPT_NETMASK		1	//subnet mask client should use (4 byte mask)
#define DHCP_OPT_ROUTERS		3	//routers client should use (IP addr list)
#define DHCP_OPT_TIMESERVERS	4	//time servers client should use (IP addr list)
#define DHCP_OPT_NAMESERVERS	5	//name servers client should use (IP addr list)
#define DHCP_OPT_DNSSERVERS		6	//DNS servers client should use (IP addr list)
#define DHCP_OPT_HOSTNAME		12	//host name client should use (string)
#define DHCP_OPT_DOMAINNAME		15	//domain name client should use (string)
#define DHCP_OPT_REQUESTEDIP	50	//IP address requested by client (IP address)
#define DHCP_OPT_LEASETIME		51	//DHCP Lease Time (uint32 seconds)
#define DHCP_OPT_DHCPMSGTYPE	53	//DHCP message type (1 byte)
#define DHCP_OPT_SERVERID		54	//Server Identifier (IP address)
#define DHCP_OPT_PARAMREQLIST	55	//Paramerter Request List (n OPT codes)
#define DHCP_OPT_RENEWALTIME	58	//DHCP Lease Renewal Time (uint32 seconds)
#define DHCP_OPT_REBINDTIME		59	//DHCP Lease Rebinding Time (uint32 seconds)
#define DHCP_OPT_CLIENTID		61	//DHCP Client Identifier
#define DHCP_OPT_END			255 //token end value (marks end of options list)
//Code for DHCP message type
#define DHCP_MSG_DHCPDISCOVER	1	//DISCOVER is broadcast by client to solicit OFFER from any/all DHCP servers.
#define DHCP_MSG_DHCPOFFER		2	//OFFER(s) are made to client by server to offer IP address and config info.
#define DHCP_MSG_DHCPREQUEST	3	//REQUEST is made my client in response to best/favorite OFFER message.
#define DHCP_MSG_DHCPDECLINE	4	//DECLINE may be sent by client to server to indicate IP already in use.
#define DHCP_MSG_DHCPACK		5	//ACK is sent to client by server in confirmation of REQUEST, contains config and IP.
#define DHCP_MSG_DHCPNAK		6	//NAK is sent to client by server to indicate problem with REQUEST.
#define DHCP_MSG_DHCPRELEASE	7	//RELEASE is sent by client to server to relinquish DHCP lease on IP address, etc.
#define DHCP_MSG_DHCPINFORM		8	//INFORM is sent by client to server to request config info, IP address configured locally.
//
#define DHCP_UDP_SERVER_PORT	67	//UDP port where DHCP requests should be sent
#define DHCP_UDP_CLIENT_PORT	68	//UDP port clients will receive DHCP replies
//
#define DHCP_TIMEOUT	10
#define DHCP_RETRIES	3

//--------------------------------------------------------------------------------------
void dhcpInit(void);
unsigned char* dhcpSetOption(unsigned char* options, unsigned char optcode, unsigned char optlen, void* optvalptr);
unsigned char dhcpGetOption(unsigned char* options, unsigned char optcode, unsigned char optlen, void* optvalptr);
#ifdef DHCP_DEBUG
void dhcpPrintHeader(struct netDhcpHeader* packet);
#endif
void dhcpDiscover(void);
void dhcpRequest(struct netDhcpHeader* packet, unsigned long serverid);
void dhcpIn(unsigned int len, struct netDhcpHeader* packet);
void dhcpRelease(void);
void dhcpTimer(void);
void dhcpService(void);

//--------------------------------------------------------------------------------------
#endif
