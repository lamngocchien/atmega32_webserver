//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef	UDP_H
#define UDP_H
//-------------------------------------------------------------------------------------
void udpSend(unsigned long dstIp, unsigned int dstPort, unsigned int srcPort, unsigned int len, unsigned char* udpData);
void UDPProcess(unsigned int len, struct ntIPHeader* packet); //__attribute__ ((weak));
//--------------------------------------------------------------------------------------
#endif //UDP_H
