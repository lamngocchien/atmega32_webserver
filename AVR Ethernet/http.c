/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include "packet.h"
#include "ethernet.h"
#include "http.h"
#include "webpage.h"
#include <math.h>
#include "tcp.h"
#include "uart.h"
#include "AVRnet.h"
prog_char Auth_str[]="Authorization";
uint8_t needreload = 0;
prog_char http_pageheader_ok[]={	"HTTP/1.0 200 Document follows\r\n"
								"Server: AVR_Small_Webserver\r\n"
								"Content-Type: text/html\r\n\r\n"};
prog_char http_pageheader_unauth[]={	"HTTP/1.0 401 Unauthorized\r\n"
								"Server: AVR_Small_Webserver\r\n"
								"WWW-Authenticate: Basic realm=\"NeedPassword\""
								"\r\nContent-Type: text/html\r\n\r\n"};
prog_char http_pageheader_auth_error[] = {"401 Unauthorized%END"};
prog_char http_error_notimp[]={	"HTTP/1.1 501 Not Implemented\r\n"
								"Server: AVR_Small_Webserver\r\n"
								"WWW-Authenticate: Basic realm=\"My AVR web server\""
								"\r\nContent-Type: text/html\r\n\r\n"};
struct httpSession	httpSessionTable[MAX_HTTP_SESSION];
unsigned char http_auth_password[20];
PROGMEM char BASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
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
void httpInit()
{
	unsigned char i;
	decode_base64((unsigned char*)HTTP_AUTH_STRING,http_auth_password);
	for(i=0; i < MAX_HTTP_SESSION; i++){
		httpSessionTable[i].status = HTTP_STATUS_IDLE;
	}
}
//Ham xu ly mot request cua giao thuc HTTP
void httpGetRequest(unsigned char *buffer,unsigned int bufferLen,struct httpRequest* rqst)
{
	unsigned int i;
	if((buffer[0] == 'G')&&(buffer[1] == 'E')&&(buffer[2] == 'T')){
	//if(findstr("GET",buffer,3) == 0){
		rqst->method = HTTP_REQUEST_GET;
	}else if((buffer[0] == 'P')&&(buffer[1] == 'O')&&(buffer[2] == 'S')&&(buffer[3] == 'T')){
	//}else if(findstr("POST",buffer,4) == 0){
		rqst->method = HTTP_REQUEST_POST;
	}else if((buffer[0] == 'H')&&(buffer[1] == 'E')&&(buffer[2] == 'A')&&(buffer[3] == 'D')){
	//}else if(findstr("HEAD",buffer,4) == 0){
		rqst->method = HTTP_REQUEST_HEAD;
	}else{
		rqst->method = HTTP_REQUEST_UNKNOWN;
	}
	//Bat dau tim URI va HTTP version
	i = 0;
	while(++i<bufferLen){
		if(buffer[i] == ' '){	
			//khoang trang dau tien
			rqst->requestURI = &buffer[i+1];
			break;
		}
	}
	while(++i<bufferLen)
		{
		if(buffer[i] == ' ')
			{	
			//khoang trang dau tien
			rqst->version = &buffer[i+1];
			break;
			}
		}
	//Tim header
	while(++i<bufferLen)
		{
		if((buffer[i] == 0x0d) && (buffer[i+1] == 0x0a))
			{	
			//\n\r dau tien, bat dau header
			rqst->header = &buffer[i+2];
			break;
			}
		}
	//Tim ket thuc header
	while(++i<bufferLen)
		{
		if((buffer[i] == 0x0d) && (buffer[i+1] == 0x0a) && (buffer[i+2] == 0x0d))
			{	
				//\n\r\n\r
				i += 4;
				break;
			}	
		}
	rqst->body = &buffer[i];
	rqst->bodyLen = bufferLen - i;
}
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

unsigned char val_tram = 0, val_chuc = 0, val_dvi = 0, val_nghin=0, val_cnghin=0;
void change_uint16 (uint16_t val)
{
	uint16_t temp;
	val_cnghin = val /10000;
	temp = val %10000;	
	val_nghin  = (temp) /1000;	
	temp = temp %1000;
	val_tram   = (temp) /100;	
	temp = temp %100;
	val_chuc   = temp/10;
	val_dvi    = temp%10;	
}
void change_number(unsigned char val)
{
	val_tram = val /100;	
	val_chuc = (val %100) /10;	
	val_dvi = (val %100) %10;
}

//Ham gui mot doan du lieu chua trong bo nho chuong trinh ra theo giao thuc HTTP
void	HTTPSend(const prog_char progdata[],unsigned int dataLen,struct tcpSession *pSession, unsigned char endData)
{
	unsigned int i;
	unsigned int srcDataIdx = 0;
	unsigned char* dataBuffer;
	unsigned char Flags;
	unsigned char tmpChr,tmpVar;
	extern volatile uint8_t	Second, Minute, Hour;
	extern uint16_t countdown_min[5];
	extern uint8_t  state_count[5], auto_off[5], auto_state[5];
	extern uint8_t ds18b20, digit;
	
	//unsigned char temp_adc;
	Flags = TCP_PSH_FLAG;

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
		while(i<MAX_SEGMENT_SIZE)
			{
			tmpChr = pgm_read_byte(progdata + srcDataIdx++);
			//Truyen nhan du lieu
			if(tmpChr == '%')
				{
					//Neu khong con du cho trong tren buffer
					if( (i + 9) > MAX_SEGMENT_SIZE)
						{
							srcDataIdx--;
							break;
						}
						
					//Tiep tuc neu du cho trong tren buffer
					tmpVar = pgm_read_byte(progdata + srcDataIdx + 3) - 0x30;
					//Truong hop tim duoc CL
					if((pgm_read_byte(progdata + srcDataIdx) == 'C') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'L'))
						{
						if(GetState(pgm_read_byte(progdata + srcDataIdx + 2) - 0x30) == 240)// Get trang thai cua tung role
							{dataBuffer[i++] = 'o';dataBuffer[i++] = 'n';}
						else
							{dataBuffer[i++] = 'o';dataBuffer[i++] = 'f'; dataBuffer[i++] = 'f';}
						srcDataIdx+=3;
						}
					else
					//Truong hop tim duoc RL
					if((pgm_read_byte(progdata + srcDataIdx) == 'R') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'S'))
						{
						if(GetState(pgm_read_byte(progdata + srcDataIdx + 2) - 0x30) == 240)// Get trang thai cua tung role
							{dataBuffer[i++] = 'O';dataBuffer[i++] = 'F';dataBuffer[i++] = 'F';}
						else
							{dataBuffer[i++] = 'O';dataBuffer[i++] = 'N';}
						srcDataIdx+=3;
						}	
					else
					//Truong hop tim duoc TI
						if((pgm_read_byte(progdata + srcDataIdx) == 'T') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'I'))
							{									
								//dataBuffer[i++] = ' '; 
								if(Hour/10 != 0)	dataBuffer[i++] = Hour/10 + 0x30;	dataBuffer[i++] = Hour%10 + 0x30;
								dataBuffer[i++]=':';
								dataBuffer[i++] = Minute/10 + 0x30;		dataBuffer[i++] = Minute%10 + 0x30;
								dataBuffer[i++]=':'; 
								dataBuffer[i++] = Second/10 + 0x30;		dataBuffer[i++] = Second%10 + 0x30;
								//dataBuffer[i++]=' ';
								//srcDataIdx+=10;
								srcDataIdx+=9;	
							}
					else
					//Truong hop tim duoc AD
					if((pgm_read_byte(progdata + srcDataIdx) == 'A') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'D'))
						{												
							//dataBuffer[i++] = ' ';
							if(ds18b20 /100 == 0 && (ds18b20 %100) /10 != 0)	dataBuffer[i++] = (ds18b20 %100) /10 + 0x30;
							else if(ds18b20 /100 != 0) {dataBuffer[i++] = ds18b20 /100 + 0x30;	dataBuffer[i++] = (ds18b20 %100) /10 + 0x30;}
							dataBuffer[i++] = (ds18b20 %100) %10 + 0x30;	
							dataBuffer[i++] = '.';
							dataBuffer[i++] = digit/10 + 0x30;
							dataBuffer[i++] = digit%10 + 0x30;
							//dataBuffer[i++] = ' ';
							srcDataIdx+=2;
						}	
					//Truong hop tim duoc CD
					else
					if((pgm_read_byte(progdata + srcDataIdx) == 'C') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'D'))
						{
							//dataBuffer[i++] = ' ';
							if(countdown_min[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30] != 0 && state_count[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30] != 255)	// Kiem tra xem co hen h hay khong
								{
								change_uint16(countdown_min[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30]);
								if(val_cnghin != 0) {dataBuffer[i++] = val_cnghin + 0x30; dataBuffer[i++] = val_nghin + 0x30; dataBuffer[i++] = val_tram + 0x30; dataBuffer[i++] = val_chuc + 0x30;}
								if(val_cnghin == 0 && val_nghin !=0) {dataBuffer[i++] = val_nghin + 0x30; dataBuffer[i++] = val_tram + 0x30; dataBuffer[i++] = val_chuc + 0x30;}
								if(val_cnghin == 0 && val_nghin ==0 && val_tram !=0 ) {dataBuffer[i++] = val_tram + 0x30; dataBuffer[i++] = val_chuc + 0x30;}	
								if(val_cnghin == 0 && val_nghin ==0 && val_tram ==0 && val_chuc != 0 ) dataBuffer[i++] = val_chuc + 0x30;
								dataBuffer[i++] = val_dvi + 0x30;	
								}
							//dataBuffer[i++] = ' ';
							srcDataIdx+=3;
						}
					//Truong hop tim duoc TS
					else
					if((pgm_read_byte(progdata + srcDataIdx) == 'T') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'S'))
						{
							//dataBuffer[i++] = ' ';
							//printf("Sent state device %d :countdown_min %d countdown_state %d and %d\r\n",pgm_read_byte(progdata + srcDataIdx + 2)-0x30,countdown_min[pgm_read_byte(progdata + srcDataIdx + 2)-0x30],state_count[pgm_read_byte(progdata + srcDataIdx + 2)-0x30],test_out_state(pgm_read_byte(progdata + srcDataIdx + 2)-0x30));
							switch(test_out_state(pgm_read_byte(progdata + srcDataIdx + 2) - 0x30))	//Kiem tra trang thai neu co hen gio
							{
								case 2:	dataBuffer[i++] = 'O';	dataBuffer[i++] = 'N';	break;
								case 1:	dataBuffer[i++] = 'O';	dataBuffer[i++] = 'F';	dataBuffer[i++] = 'F';	break;
								//case 0:	dataBuffer[i++] = ' ';	dataBuffer[i++] = ' ';	break;	//Khong co hen gio
								default: break;
							}
							//dataBuffer[i++] = ' ';
							srcDataIdx+=3;
						}
					//Truong hop tim duoc TA
					else
					if((pgm_read_byte(progdata + srcDataIdx) == 'T') && (pgm_read_byte(progdata + srcDataIdx + 1) == 'A'))
						{
												
							//dataBuffer[i++] = ' ';
							if(auto_off[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30] != 0)
								{
								change_number(auto_off[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30]);
								if(auto_state[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30] == 240) dataBuffer[i++] = 'H';
								else if(auto_state[pgm_read_byte(progdata + srcDataIdx + 2) - 0x30] == 15) dataBuffer[i++] = 'L';
								dataBuffer[i++] = ' ';
								if(val_tram== 0 && val_chuc != 0)	dataBuffer[i++] = val_chuc + 0x30;
								else if(val_tram != 0) {dataBuffer[i++] = val_tram + 0x30;	dataBuffer[i++] = val_chuc + 0x30;}
								dataBuffer[i++] = val_dvi + 0x30;											
								}
							//dataBuffer[i++] = ' ';
							srcDataIdx+=3;
						}						
					//Cac truong hop con lai
					else
						dataBuffer[i++] = tmpChr;																																					
				}
			else
				dataBuffer[i++] = tmpChr;	//Copy data to tcp data buffer
			if(srcDataIdx==dataLen)
				break;
		}
		Flags |= TCP_ACK_FLAG;
		if(endData){
			if(srcDataIdx == dataLen){
				Flags |= TCP_FIN_FLAG;
				pSession->sesState = TCP_STATE_FIN_WAIT1;
			}
		}
		TCPPackedSend(pSession,Flags,i,dataBuffer);
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
//Cac ham xu ly du lieu nhan duoc
unsigned char mData[10], index;
unsigned int time_dev;
unsigned char  state_dev, temp_dev;

//Ham lay gia tri thoi gian
void get_time()
{
	uint8_t temp = 3;
	state_dev = 240;
	if(mData[2] == 70)	{ temp++;	state_dev = 15;}
	time_dev = 0;
	for( uint8_t i = temp; i <= index; i++)
		{
		time_dev = time_dev + (mData[i] - 48)*pow(10,index-i);
		if(index - i == 2) time_dev++;
		}
}
//Ham lay gia tri nhiet do
void get_temp()
{
	uint8_t temp = index - 2;
	state_dev = 240;
	if(mData[index] == 70)	{temp--;	state_dev = 15;}
	temp_dev = 0;
	for(uint8_t i = 1; i <= temp; i++)
		{
			temp_dev+=(mData[i] - 48)*pow(10,temp - i);
			if(temp - i == 2) temp_dev++;
		}
}
void apply_edit() // HMS & DMY
{
	uint8_t val1, val2, val3, type;
	if(mData[3] == 83) type = 1;
	else if(mData[3] == 89) type = 2;
	val1 = (mData[4] - 48)*10 + (mData[5] - 48);
	val2 = (mData[6] - 48)*10 + (mData[7] - 48); 
	val3 = (mData[8] - 48)*10 + (mData[9] - 48);
	if(type != 0)	edit_clock(type, val1, val2, val3);
}
//Lay gia tri chuoi auto_device trong buffer
void get_string(unsigned char* str,unsigned int len,unsigned int device)
{
	unsigned int i = 0;
	unsigned int j = 1;
	for(unsigned char k = 1; k <= 16; k++)	mData[k] = 0;
	while (i<len)
	{
		if(str[i] == 'A' && str[i+1] == 'u' && str[i+2] == 't' && str[i+3] == 'o' && str[i+4] == device + 0x30) // Bang cac gia tri config_auto
			{
				i += 6;
				index = 0;
				while(str[i] != '&')
					{
						mData[j++] = str[i++];
						index++;
					}	
				break;
			}
		i++;
	}
	if(mData[1] == 79)	get_time();
	else if(mData[index] == 70 || mData[index] == 78)	get_temp();
	else if(mData[2] == 77)  apply_edit();
	else if(mData[2] == 65)	 edit_day(mData[4] - 48);
}

//Ham xu ly mot goi thuoc giao thuc HTP nhan duoc
void httpDataIn(unsigned char *buffer,unsigned int bufferLen,struct tcpSession *pSession)
{
	unsigned char i;
	unsigned char *tmpstr;
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
				httpSessionTable[i].auth = 1;
			//Neu xac thuc khong hop le
			}else{
				HTTPSend(http_pageheader_unauth,sizeof(http_pageheader_unauth)-1,pSession,0);
				HTTPSend(http_pageheader_auth_error,sizeof(http_pageheader_auth_error)-1,pSession,1);
				httpSessionTable[i].status = HTTP_STATUS_IDLE;
				return;
			}
		}
		//Process for each method
		//If GET method
		if(httpSessionTable[i].rqst.method == HTTP_REQUEST_GET){
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
		}else 
			if(httpSessionTable[i].rqst.method == HTTP_REQUEST_POST)
			{
			if(findstr(PSTR("RL1=ON"),buffer,bufferLen)	!= -1)	{ON1	AUTO1	needreload = 1;}	else if(findstr(PSTR("RL1=OFF"),buffer,bufferLen) != -1)	{OFF1	AUTO1	needreload = 1;}
			if(findstr(PSTR("RL2=ON"),buffer,bufferLen)	!= -1)	{ON2	AUTO2	needreload = 1;}	else if(findstr(PSTR("RL2=OFF"),buffer,bufferLen) != -1)	{OFF2	AUTO2	needreload = 1;}
			if(findstr(PSTR("RL3=ON"),buffer,bufferLen)	!= -1)	{ON3	AUTO3	needreload = 1;}	else if(findstr(PSTR("RL3=OFF"),buffer,bufferLen) != -1)	{OFF3	AUTO3	needreload = 1;}
			if(findstr(PSTR("RL4=ON"),buffer,bufferLen)	!= -1)	{ON4	AUTO4	needreload = 1;}	else if(findstr(PSTR("RL4=OFF"),buffer,bufferLen) != -1)	{OFF4	AUTO4	needreload = 1;}
			//Update POST data
			if(findstr(PSTR("SUB=Submit"),buffer,bufferLen) != -1)
				{
				needreload = 1;
				if(findstr(PSTR("Auto1=reset"),buffer,bufferLen) != -1) reset_one_device(1);
				if(findstr(PSTR("Auto1=rsall"),buffer,bufferLen) != -1) reset_all();
				if(findstr(PSTR("Auto2=reset"),buffer,bufferLen) != -1) reset_one_device(2);
				if(findstr(PSTR("Auto3=reset"),buffer,bufferLen) != -1) reset_one_device(3);
				if(findstr(PSTR("Auto4=reset"),buffer,bufferLen) != -1) reset_one_device(4);	
				//Get String	
				for(uint8_t m = 1 ; m <= 4 ; m++)				
						{
						state_dev = time_dev = index = temp_dev=0;
						get_string(buffer,bufferLen,m);
						if(time_dev != 0)	{set_time_count(m,state_dev,time_dev);	/*printf("Set Time: \r\nState  = %d \r\nTime   = %d \r\nDevice = %d\r\n",state_dev,time_dev,m);*/}
						else
						if(temp_dev != 0)	{set_temp_val(temp_dev,state_dev,m);	/*printf("Set Temp: \r\nState  = %d \r\nTemp   = %d \r\nDevice = %d\r\n ",state_dev,temp_dev,m);*/}
						}					
				}
				
			//Send HTTP	
			if(needreload)
			{
			needreload = 0;
			HTTPSend(http_pageheader_ok,sizeof(http_pageheader_ok)-1,pSession,0);
			HTTPSend(Page1,sizeof(Page1)-1,pSession,1);
			httpSessionTable[i].status = HTTP_STATUS_IDLE;
			return;	
			}			
			else	HTTPSend("",0,pSession,0);
		}
	}
}
