/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef ICMP_H
#define ICMP_H
#include "packet.h"
void icmpIpIn(struct ntIPHeader* ipHeader);
void icmpEchoReply(struct ntIPHeader* ipHeader);
void icmpPrintHeader(struct ntIPHeader* ipHeader);
#endif //ICMP_H
