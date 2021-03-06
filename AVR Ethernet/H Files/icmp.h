//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
//======================================================================================
//									icmp.h
//======================================================================================
//	This is header file for icmp.c
//		Writen by NTTam
//		PTITHCM
//		Ver 1.0
//======================================================================================
#ifndef ICMP_H
#define ICMP_H
//--------------------------------------------------------------------------------------
#include "packet.h"
//--------------------------------------------------------------------------------------
void icmpIpIn(struct ntIPHeader* ipHeader);
void icmpEchoReply(struct ntIPHeader* ipHeader);
void icmpPrintHeader(struct ntIPHeader* ipHeader);

#endif //ICMP_H
