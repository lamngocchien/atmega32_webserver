//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include "packet.h"
#include "ethernet.h"
#include "http.h"
#include "webpage.h"
#include "tcp.h"////
#include "uart.h"
#include "ntAVRnet.h"
//----------------------------------------------------------------------------
prog_char http_pageheader_ok[]={	"HTTP/1.0 200 Document follows\r\n"
								"Server: AVR_Small_Webserver\r\n"
								"Content-Type: text/html\r\n\r\n"};
//
prog_char http_pageheader_unauth[]={	"HTTP/1.0 401 Unauthorized\r\n"
								"Server: AVR_Small_Webserver\r\n"
								"WWW-Authenticate: Basic realm=\"NeedPassword\""
								"\r\nContent-Type: text/html\r\n\r\n"};
//
prog_char http_pageheader_auth_error[] = {"401 Unauthorized%END"};
//
prog_char http_error_notimp[]={	"HTTP/1.1 501 Not Implemented\r\n"
								"Server: AVR_Small_Webserver\r\n"
								"WWW-Authenticate: Basic realm=\"My AVR web server\""
								"\r\nContent-Type: text/html\r\n\r\n"};
//
struct httpSession	httpSessionTable[MAX_HTTP_SESSION];
//
unsigned char http_auth_password[20];
//----------------------------------------------------------------------------
PROGMEM char BASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
//
void decode_base64 (unsigned char *str1_in,unsigned char *str2_out)
{
	unsigned char *strp;
    unsigned char end_byte = 0;
	
	strp = str2_out;
	
	while (*str1_in != 0)
	{
		*str2_out++= (*str1_in & 0xFC)>>2;
		
		*str2_out = (((*str1_in++)&0x03)<<4); 
		if (*str1_in==0)
		{
			str2_out++;
            end_byte = 2;
			break;
		}
		else
		{
			*str2_out++ += ((*str1_in & 0xF0)>>4);
			*str2_out = (((*str1_in++)& 0x0F)<<2);
			if (*str1_in==0)
			{
				str2_out++;
                end_byte = 1;
                break;
			}
			else
			{
				*str2_out++	+= ((*str1_in & 0xC0)>>6);
				*str2_out++= *str1_in++ & 0x3F;
			}
		}
	}
	*str2_out = 0;
    
	while(strp != str2_out)
	{
			*strp = pgm_read_byte(&BASE64CODE[(*strp) & 0x3F]);
			strp++;
	}
 
    while (end_byte--)
    {
        *strp = '=';
        strp++;
    }
    *strp = 0; 
}
//----------------------------------------------------------------------------
void httpInit()
{
	unsigned char i;
	decode_base64((unsigned char*)HTTP_AUTH_STRING,http_auth_password);
	for(i=0; i < MAX_HTTP_SESSION; i++){
		httpSessionTable[i].status = HTTP_STATUS_IDLE;
	}
	//UpdateCtrlSts(0x00);
}
//----------------------------------------------------------------------------
//Ham xu ly mot request cua giao thuc HTTP
void httpGetRequest(unsigned char *buffer,unsigned int bufferLen,struct httpRequest* rqst)
{
	unsigned int i;
	if((buffer[0] == 'G')&&(buffer[1] == 'E')&&(buffer[2] == 'T')){
	//if(findstr("GET",buffer,3) == 0){
		rqst->method = HTTP_REQUEST_GET;
		#ifdef HTTP_DEBUG
		printf("GET\r\n");
		#endif
	}else if((buffer[0] == 'P')&&(buffer[1] == 'O')&&(buffer[2] == 'S')&&(buffer[3] == 'T')){
	//}else if(findstr("POST",buffer,4) == 0){
		rqst->method = HTTP_REQUEST_POST;
		#ifdef HTTP_DEBUG
		printf("POST\r\n");
		#endif
	}else if((buffer[0] == 'H')&&(buffer[1] == 'E')&&(buffer[2] == 'A')&&(buffer[3] == 'D')){
	//}else if(findstr("HEAD",buffer,4) == 0){
		rqst->method = HTTP_REQUEST_HEAD;
		#ifdef HTTP_DEBUG
		printf("HEAD\r\n");
		#endif
	}else{
		rqst->method = HTTP_REQUEST_UNKNOWN;
		#ifdef HTTP_DEBUG
		printf("UNKNOWN\r\n");
		#endif
	}
	//Bat dau tim URI va HTTP version
	i = 0;
	while(++i<bufferLen){
		if(buffer[i] == ' '){	//khoang trang dau tien
			rqst->requestURI = &buffer[i+1];
			break;
		}
	}
	while(++i<bufferLen){
		if(buffer[i] == ' '){	//khoang trang dau tien
			rqst->version = &buffer[i+1];
			break;
		}
	}
	//Tim header
	while(++i<bufferLen){
		if((buffer[i] == 0x0d) && (buffer[i+1] == 0x0a)){	//\n\r dau tien, bat dau header
			rqst->header = &buffer[i+2];
			break;
		}
	}
	//Tim ket thuc header
	while(++i<bufferLen){
		if((buffer[i] == 0x0d) && (buffer[i+1] == 0x0a) && (buffer[i+2] == 0x0d)){	//\n\r\n\r
			i += 4;
			break;
		}
	}
	rqst->body = &buffer[i];
	rqst->bodyLen = bufferLen - i;

	#ifdef HTTP_DEBUG
	printf("Buffer Len:%d\r\n",bufferLen);
	printf("URI start:%d\r\n",(rqst->requestURI - buffer));
	printf("HTTP Version:%d\r\n",(rqst->version - buffer));
	printf("Header:%d\n\r",(rqst->header - buffer));
	printf("Body:%d\n\r",(unsigned int)(rqst->body - buffer));
	printf("Body:%x-%x\n\r",(unsigned int)(rqst->body),(unsigned int)buffer);
	printf("Body i:%d\n\r",i);
	#endif
}
//----------------------------------------------------------------------------
//Ham lay gia tri mot truong trong HTTP Header,
// tra lai con tro vi tri bat dau value cua truong nay
unsigned char * httpHeaderGetField(const prog_char fieldname[],struct httpRequest *rqst)
{
	unsigned char *header;
	unsigned int i,j,headerLen;
	header = rqst->header;
	headerLen = (rqst->header) - (rqst->body) - 4;
	i=0;
	j=0;
	while(i<headerLen){
		if(header[i++] != pgm_read_byte(fieldname + j++)){
			j = 0;
		}
		if(pgm_read_byte(fieldname + j) == 0){
			return(header+i+2);
		}
	}
	return(0);
}
//----------------------------------------------------------------------------
//Ham gui mot doan du lieu chua trong bo nho chuong trinh ra theo giao thuc HTTP
void	HTTPSend(const prog_char progdata[],unsigned int dataLen,struct tcpSession *pSession, unsigned char endData)
{
	unsigned int i;
	unsigned int srcDataIdx = 0;
	unsigned char* dataBuffer;
	unsigned char Flags;
	unsigned char tmpChr;

	Flags = TCP_PSH_FLAG;
	dataBuffer = ethGetBuffer() + ETH_HEADER_LEN + IP_HEADER_LEN + TCP_HEADER_LEN;
	if(dataLen == 0){
		Flags |= TCP_ACK_FLAG;
		if(endData){
			if(srcDataIdx == dataLen){
				Flags |= TCP_FIN_FLAG;
				pSession->sesState = TCP_STATE_FIN_WAIT1;
			}
		}
		TCPPackedSend(pSession,Flags,0,dataBuffer);
	}
	while(srcDataIdx < dataLen){
		i = 0;
		while(i<MAX_SEGMENT_SIZE){
			tmpChr = pgm_read_byte(progdata + srcDataIdx++);
			dataBuffer[i++] = tmpChr;	//Copy data to tcp data buffer
			if(srcDataIdx==dataLen){
				break;
			}
		}
		Flags |= TCP_ACK_FLAG;
		if(endData){
			if(srcDataIdx == dataLen){
				Flags |= TCP_FIN_FLAG;
				pSession->sesState = TCP_STATE_FIN_WAIT1;
			}
		}
		TCPPackedSend(pSession,Flags,i,dataBuffer);
		//delay_ms(100);
		#ifdef NETSTACK_DEBUG
		printf("Sent %d byte\r\n",srcDataIdx);
		#endif
	}
}
//----------------------------------------------------------------------------
unsigned int findstr(const prog_char progstr[],unsigned char* str,unsigned int len)
{
	unsigned int i,j;
	i=0;
	j=0;
	while(i<len){
		if(str[i++] != pgm_read_byte(progstr + j++)){
			j = 0;
		}
		if(pgm_read_byte(progstr + j) == 0){
			return(i-j);
		}
	}
	return(-1);
}
//----------------------------------------------------------------------------
unsigned int findstrdatamem(unsigned char* str1,unsigned char* str2,unsigned int len)
{
	unsigned int i,j;
	i=0;
	j=0;
	while(i<len){
		if(str2[i++] != str1[j++]){
			j = 0;
		}
		if(str1[j] == 0){
			return(i-j);
		}
		if(str2[i] == 0){
			return(-1);
		}
	}
	return(-1);
}
//----------------------------------------------------------------------------
//Ham xu ly mot goi thuoc giao thuc HTP nhan duoc
void httpDataIn(unsigned char *buffer,unsigned int bufferLen,struct tcpSession *pSession)
{
	unsigned char i;
	unsigned char *tmpstr;
	prog_char Auth_str[]="Authorization";
	//Tim xem cophien HTTP service da co cho phien TCP nay khong
	for(i=0; i < MAX_HTTP_SESSION; i++){
		if((httpSessionTable[i].status != HTTP_STATUS_IDLE) && (httpSessionTable[i].pTCPSession == pSession))
			break;
	}
	if(i == MAX_HTTP_SESSION){
		//Tim 1 phien trong
		for(i=0; i < MAX_HTTP_SESSION; i++){
			if(httpSessionTable[i].status == HTTP_STATUS_IDLE){
				httpSessionTable[i].pTCPSession = pSession;
				break;
			}
		}
	}
	if(httpSessionTable[i].status == HTTP_STATUS_IDLE){
	//Neu day la 1 HTTP request moi
		//Kiem tra method cua request
		httpGetRequest(buffer,bufferLen,&(httpSessionTable[i].rqst));
		if(httpSessionTable[i].rqst.method == HTTP_REQUEST_UNKNOWN){
		//Neu request khong biet, thoat tro ve trang thai idle
			httpSessionTable[i].status = HTTP_STATUS_IDLE;
			return;
		}else{
		//Set trang thai cua HTTP session thanh HEADER_RECEIVED (da nhan header)
			httpSessionTable[i].status = HTTP_STATUS_HEADER_RECEIVED;
		}
	}
	//Neu da nhan header
	if(httpSessionTable[i].status == HTTP_STATUS_HEADER_RECEIVED){
		//Kiem tra xem da xac thuc hay chua
		if(httpSessionTable[i].auth != 1){
			tmpstr = httpHeaderGetField(Auth_str,&(httpSessionTable[i].rqst));
			if(findstrdatamem(http_auth_password,tmpstr,httpSessionTable[i].rqst.body-tmpstr) != -1){
				#ifdef HTTP_DEBUG
				printf("Auth OK\n\r");
				printfStr(http_auth_password);
				printf("Input\n\r");
				printfStr(tmpstr);
				#endif
				httpSessionTable[i].auth = 1;
			//Neu xac thuc khong hop le
			}else{
				#ifdef HTTP_DEBUG
				printf("Auth fail\n\r");
				printf("Auth string:");
				printfStr(http_auth_password);
				printf("\n\r");
				printf("Received string:");
				printfStrLen(tmpstr,0,20);
				#endif
				HTTPSend(http_pageheader_unauth,sizeof(http_pageheader_unauth)-1,pSession,0);
				HTTPSend(http_pageheader_auth_error,sizeof(http_pageheader_auth_error)-1,pSession,1);
				httpSessionTable[i].status = HTTP_STATUS_IDLE;
				return;
			}
		}
		//Process for each method
		//If GET method
		if(httpSessionTable[i].rqst.method == HTTP_REQUEST_GET){
			#ifdef HTTP_DEBUG
			printf("GET HEADER:\n\r");
			printfStrLen(httpSessionTable[i].rqst.header,0
					,(httpSessionTable[i].rqst.body - httpSessionTable[i].rqst.header) + httpSessionTable[i].rqst.bodyLen);
			#endif
			//If GET the main website
			if((httpSessionTable[i].rqst.requestURI[0] == '/') && (httpSessionTable[i].rqst.requestURI[1] == ' ')){
				HTTPSend(http_pageheader_ok,sizeof(http_pageheader_ok)-1,pSession,0);
				HTTPSend(Page1,sizeof(Page1)-1,pSession,1);
				httpSessionTable[i].status = HTTP_STATUS_IDLE;
			//If error
			}else{
				HTTPSend(http_error_notimp,sizeof(http_error_notimp),pSession,1);
				httpSessionTable[i].status = HTTP_STATUS_IDLE;
			}
			return;
		//If POST method
		}else if(httpSessionTable[i].rqst.method == HTTP_REQUEST_POST){
			//Update POST data
			if(findstr(PSTR("SUB=Submit"),buffer,bufferLen) != -1){
				//Send HTTP data
				HTTPSend(http_pageheader_ok,sizeof(http_pageheader_ok)-1,pSession,0);
				HTTPSend(Page1,sizeof(Page1)-1,pSession,1);
				httpSessionTable[i].status = HTTP_STATUS_IDLE;
				return;
			}else if(findstr(PSTR("APPLY=Apply"),buffer,bufferLen) != -1){
				HTTPSend(http_pageheader_ok,sizeof(http_pageheader_ok)-1,pSession,0);
				HTTPSend(Page1,sizeof(Page1)-1,pSession,1);
				httpSessionTable[i].status = HTTP_STATUS_IDLE;
			}else{
				HTTPSend("",0,pSession,0);
			}
		}
	}
}
//----------------------------------------------------------------------------
