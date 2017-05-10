/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#ifndef HTTP_H
#define HTTP_H
//
#include <avr/pgmspace.h>
//
#define	HTTP_REQUEST_GET		1//"GET"
#define	HTTP_REQUEST_POST		2//"POST"
#define	HTTP_REQUEST_HEAD		3//"HEAD"
#define	HTTP_REQUEST_UNKNOWN	4//Request Unknow
#define	HTTP_REQUEST_GET_STR		"GET"
#define	HTTP_REQUEST_POST_STR		"POST"
#define	HTTP_REQUEST_HEAD_STR		"HEAD"
//
struct httpRequest{
	unsigned char method;		//end with space
	unsigned char *requestURI;	//end with space
	unsigned char *version;		//end with \n\r
	unsigned char *header;		//end with \n\r\n\r
	unsigned char *body;		//
	unsigned int bodyLen;
};
//
struct httpSession{
	unsigned char status;			//Trang thai hien tai
	const prog_char *headerPointer;	//Con tro den header trong bo nho chuong trinh
	const prog_char *bodyPointer;	//Con tro den du lieu can gui trong bo nho chuong trinh
	unsigned char *authStr;			//Con tro den chuoi xac thuc
	unsigned char auth;				//Xac thuc
	struct httpRequest rqst;		//Request tuong ung
	struct tcpSession *pTCPSession;
};
//
#define HTTP_STATUS_IDLE			0
#define HTTP_STATUS_GOT_REQUEST		1
#define HTTP_STATUS_HEADER_SENT		2
#define HTTP_STATUS_HEADER_RECEIVED	3
#define HTTP_STATUS_FINISH			4
//
#define MAX_HTTP_SESSION			8
//
void httpDataIn(unsigned char *buffer,unsigned int bufferLen,struct tcpSession *pSession);
#endif //HTTP_H
