/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef PACKET_H
#define PACKET_H

//Dia chi vat ly hay dia chi MAC (lop Ethernet)
struct ntEthAddr
{
	unsigned char addr[6];
};

//Ethernet header
//  Gom 14 byte:
//  06 byte dia chi dich
//  06 byte dia chi nguon
//  02 byte type (cho biet frame ethernet mang ben trong loai du lieu gi)
#define ETH_HEADER_LEN	14
struct ntEthHeader
{
	struct ntEthAddr desAddr;
	struct ntEthAddr srcAddr;
	unsigned int type;
};
//
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IP  0x0800
#define ETH_TYPE_IP6 0x86dd

//Ethernet header 802.1q VLAN Tagging
struct ntEth802_1qHeader
{
	struct ntEthAddr desAddr;
	struct ntEthAddr srcAddr;
	unsigned int type;
	unsigned int TPID;
	unsigned int PCP_CFI_VID;
};
#define ETH_802_1Q_HEADER_LEN	18
//
#define ETH_802_1Q_TPID			0x8100
#define ETH_802_1Q_PCP_MASK		0xE000
#define ETH_802_1Q_CFI_MASK		0x1000
#define ETH_802_1Q_VID_MASK		0x0FFF

//Ethernet header 802.1ad Q-in-Q VLAN Tagging
struct ntEth802_1adHeader
{
	struct ntEthAddr desAddr;
	struct ntEthAddr srcAddr;
	unsigned int type;
	unsigned int OuterTPID;
	unsigned int OuterPCP_CFI_VID;
	unsigned int InnerTPID;
	unsigned int InnerPCP_CFI_VID;
};
#define ETH_802_1AD_HEADER_LEN		22
//
#define ETH_802_1AD_TPID			0x88a8
#define ETH_802_QINQ_TPID1			0x9100
#define ETH_802_QINQ_TPID2			0x9200
#define ETH_802_QINQ_TPID3			0x9300

//Cau truc MPLS Header
struct ntMPLSHeader
{
	unsigned int	HighLabelValue;
	unsigned char	TrafficClass_Stack;
	unsigned char	TTL;
};
#define MPLS_HEADER_LEN			4
//
#define MPLS_LOW_LABEL_MASK		0xF0
#define MPLS_TRF_CLS_MASK		0x0E
#define MPLS_STACK_MASK			0x01
//

//Cau truc IP header
struct ntIPHeader
{
	unsigned char	verHdrLen;
	unsigned char	ToS;
	unsigned int	Len;
	unsigned int	IDNumber;
	unsigned int	Offset;
	unsigned char	TTL;
	unsigned char	Protocol;
	unsigned int	Checksum;
	unsigned long	srcIPAddr;
	unsigned long	desIPAddr;
	unsigned char	Option[4];
};
#define IP_HEADER_LEN	20

#define IP_PROTO_ICMP	1
#define IP_PROTO_TCP	6
#define IP_PROTO_UDP	17

//Cau truc ARP header
struct ntARPHeader
{
	unsigned int	hwType;
	unsigned int	protocol;
	unsigned char	hwLen;
	unsigned char	protoLen;
	unsigned int	opcode;
	struct ntEthAddr shwaddr;
	unsigned long	sipaddr;
	struct ntEthAddr dhwaddr;
	unsigned long	dipaddr; 
};
#define ARP_OPCODE_REQUEST	1
#define ARP_OPCODE_REPLY	2
#define ARP_HWTYPE_ETH		1

#define HTONS(s)		((s<<8) | (s>>8))	//danh cho bien 2 byte
#define HTONL(l)		((l<<24) | ((l&0x00FF0000l)>>8) | ((l&0x0000FF00l)<<8) | (l>>24))	//danh cho bien 4 byte
//

//Cau truc ICMP header
struct ntICMPHeader
{
	unsigned char	Type;
	unsigned char	Code;
	unsigned int	Checksum;
	unsigned int	ID;
	unsigned int	seqNumber;
};
#define ICMP_HEADER_LEN	8
#define ICMP_TYPE_ECHOREPLY		0
#define ICMP_TYPE_ECHOREQUEST	8
//

//Cau truc TCP header
struct ntTCPHeader
{
	unsigned int	srcPort;
	unsigned int	desPort;
	unsigned long	seqNumber;
	unsigned long	ackNumber;
	unsigned char	Offset;
	unsigned char	Flags;
	unsigned int	Window;
	unsigned int	Checksum;
	unsigned int	UrgentPtr;
	unsigned char	optdata[8];
};
#define TCP_HEADER_LEN	20

//
#define TCP_NON_FLAG (0)
#define TCP_FIN_FLAG (1<<0)
#define TCP_SYN_FLAG (1<<1)
#define TCP_RST_FLAG (1<<2)
#define TCP_PSH_FLAG (1<<3)
#define TCP_ACK_FLAG (1<<4)
#define TCP_URG_FLAG (1<<5)
#define TCP_ECE_FLAG (1<<6)
#define TCP_CWR_FLAG (1<<7)
//

//Cau truc UDP header
struct ntUDPHeader
{
	unsigned int	srcPort;
	unsigned int	desPort;
	unsigned int	Len;
	unsigned int	Checksum;
};
#define UDP_HEADER_LEN	8
//
#endif //PACKET_H
