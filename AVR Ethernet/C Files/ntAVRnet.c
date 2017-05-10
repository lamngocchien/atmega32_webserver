//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include <avr/io.h>
#include "ethernet.h"
#include "ip.h"
#include "arp.h"
#include "timer.h"
#include "uart.h"
#include "dhcp.h"////
#include "tcp.h"/////
#include "http.h"/////
#include "ntAVRnet.h"

extern struct ipConfig IpMyConfig;	///< Local IP address/config structure

//----------------------------------------------------------------------------
void netInit(unsigned long ipaddress, unsigned long netmask, unsigned long gatewayip)
{
	// init network device driver
	#ifdef NETSTACK_DEBUG
	printf("Initializing Network Device\r\n");
	#endif
	ethInit();
	// init ARP
	#ifdef NETSTACK_DEBUG
	printf("Initializing ARP cache\r\n");
	#endif
	arpInit();
	// init addressing
	#ifdef NETSTACK_DEBUG
	printf("Initializing Addressing\r\n");
	#endif
	ipSetConfig(ipaddress, netmask, gatewayip);
}
//--------------------------------------------------------------------------------------
void	PrintIPConfig()
{
	printf("MAC Address: ");		ethPrintAddr(&IpMyConfig.ethaddr);		printf("\n\r");
	printf("IP  Address: ");		ipPrintAddr(IpMyConfig.ip);				printf("\n\r");
	printf("Subnet Mask: ");		ipPrintAddr(IpMyConfig.netmask);		printf("\n\r");
	printf("Default Gateway: ");	ipPrintAddr(IpMyConfig.gateway);		printf("\n\r");
}
//--------------------------------------------------------------------------------------
void	SystemInit()
{
	timerInit();
	uartInit(UART_BAUDRATE);
	//timerPause(100);
	

}
//--------------------------------------------------------------------------------------
int	main()
{
	SystemInit();
	printf("\r\nNTTam AVR network testing with enc28j60.\r\n");
	printf("Initializing Network Interface and Stack\r\n");
	printf("Ethernet chip init\r\n");
	IpMyConfig.ethaddr.addr[0] = ETHADDR0;
	IpMyConfig.ethaddr.addr[1] = ETHADDR1;
	IpMyConfig.ethaddr.addr[2] = ETHADDR2;
	IpMyConfig.ethaddr.addr[3] = ETHADDR3;
	IpMyConfig.ethaddr.addr[4] = ETHADDR4;
	IpMyConfig.ethaddr.addr[5] = ETHADDR5;
	IpMyConfig.ip = IPADDRESS;
	IpMyConfig.netmask = NETMASK;
	IpMyConfig.gateway = GATEWAY;
	netInit(IpMyConfig.ip, IpMyConfig.netmask, IpMyConfig.gateway);
	PrintIPConfig();
	printf("Getting IP Address....\r\n");
	if(IpMyConfig.ip == 0x00000000){
		dhcpInit();
	}
	TCPCreateSession(80,httpDataIn);/////

	while(1)
	{
		ethService();
		dhcpService();
	}
	return 0;
}

