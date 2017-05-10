//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#include "packet.h"
#include "ethernet.h"
#include "ip.h"
#include "uart.h"
#include "tcp.h"
//----------------------------------------------------------------------------
struct tcpSession tcpSessionTable[TCP_MAX_SESSION];

//--------------------------------------------------------------------------------------
//Ham khoi tao so tuan tu cho mot phien TCP
// Hien tai su dung gia tri 1234 (may tinh thuong dung gia tri thoi gian hien tai)
unsigned long TCPInitSequenceNumber()
{
	return(1234);
}

//--------------------------------------------------------------------------------------
//Ham dong mot phien TCP
void TCPCloseSession(unsigned char socketnum)
{
	tcpSessionTable[socketnum].sesState = TCP_STATE_CLOSED;
	#ifdef TCP_DEBUG
	printf("Close TCP session %d\r\n",socketnum);
	#endif
}

//--------------------------------------------------------------------------------------
//Khoi dong cac gia tri trong bang TCP session
void TCPInit()
{
	unsigned char i = 0;
	for(i=0; i<TCP_MAX_SESSION; i++){
		TCPCloseSession(i);
	}
}

//--------------------------------------------------------------------------------------
//Tim mot session TCP dang roi
unsigned char TCPGetFreeSession(){
	unsigned char i;
	for(i=0; i<TCP_MAX_SESSION; i++){
		if (tcpSessionTable[i].sesState == TCP_STATE_CLOSED)
			return i;
	}
	//no free closed socket fount! -> kick an TIMED_WAIT socket
	for(i=0; i<TCP_MAX_SESSION; i++){
		if (tcpSessionTable[i].sesState == TCP_STATE_TIMED_WAIT){
			TCPCloseSession(i);
			return i;
		}
	}
	//no more free sockets ... return invalid val	
	return(TCP_MAX_SESSION);
}

//--------------------------------------------------------------------------------------
//Ham khoi tao mot session TCP o che do server de cho ket noi
void	TCPCreateSession(unsigned int  sourcePort, prog_void* appService)
{
	unsigned char i;
	i = TCPGetFreeSession();
	if(i >= TCP_MAX_SESSION)
		i = 0;	//force session 0
	tcpSessionTable[i].srcPort = sourcePort;
	tcpSessionTable[i].sesState = TCP_STATE_LISTEN;		//Current state of TCP session
	tcpSessionTable[i].srcWin = 8192;//NETSTACK_BUFFERSIZE - ETH_HEADER_LEN - IP_HEADER_LEN - TCP_HEADER_LEN - 16;
	tcpSessionTable[i].desWin = tcpSessionTable[i].srcWin;
	tcpSessionTable[i].timeOut = TCP_TIMEOUT;		//Session time out
	tcpSessionTable[i].appDataIn = appService;
	#ifdef TCP_DEBUG
	printf("TCP session created: %d\r\n", i);
	#endif

}

//--------------------------------------------------------------------------------------
//Duoc goi moi giay de kiem tra Time out cho cac phien TCP,
// giai phong cac phine TCP bi treo
void TCPCheckTimeOut(){
	unsigned char i;
	for(i=0; i<TCP_MAX_SESSION; i++){
		//decrement ttl:
		if ((tcpSessionTable[i].sesState != TCP_STATE_CLOSED) && (tcpSessionTable[i].sesState != TCP_STATE_LISTEN)){
			if(tcpSessionTable[i].timeOut)
				tcpSessionTable[i].timeOut--;
			
			//if socket TTL count is zero, close this socket!
			if (tcpSessionTable[i].timeOut == 0){
				TCPCloseSession(i);
			}
		}
	}
}

//--------------------------------------------------------------------------------------
//Tinh checksum cho goi TCP
unsigned int checksum(unsigned char *buffer, unsigned int len, unsigned long csum32)
{
	unsigned int  res16 = 0x0000;
	unsigned char data_hi;
	unsigned char data_lo;
	while(len > 1){
		data_hi = *buffer++;
		data_lo = *buffer++;
		res16 = (((unsigned int)data_hi << 8) + data_lo);
		csum32 = csum32 + res16;
		len -=2;
	}
	if(len > 0){
		data_hi = *buffer;
		res16   = (unsigned int)data_hi<<8;
		csum32 = csum32 + res16;
	}
	while(csum32>>16)
		csum32 = (csum32 & 0xFFFF)+ (csum32 >> 16);
	//csum32 = ((csum32 & 0x0000FFFF)+ ((csum32 & 0xFFFF0000) >> 16));	
	res16  =~(csum32 & 0x0000FFFF);
	return (res16);
}
//--------------------------------------------------------------------------------------
//Gui di mot goi TCP
void TCPPackedSend(struct tcpSession *pSession, unsigned char Flags, unsigned int len, unsigned char *dataBuffer)
{
	unsigned int tmp;
	unsigned long checksum32;
	//Make pointer to TCP header
	struct ntTCPHeader* tcpHeader;
	struct ntIPHeader* ipHeader;
	//Neu dang syn thi them option ve MSS
	if(Flags & TCP_SYN_FLAG){
		//Option data
		dataBuffer[0] = 0x02;
		dataBuffer[1] = 0x04;
		dataBuffer[2] = (MAX_SEGMENT_SIZE >> 8) & 0xff;
		dataBuffer[3] = MAX_SEGMENT_SIZE & 0xff;
		dataBuffer[4] = 0x01;
		dataBuffer[5] = 0x03;
		dataBuffer[6] = 0x03;
		dataBuffer[7] = 0x00;
		//Move data pointer to make room for TCP header
	}
	dataBuffer -= TCP_HEADER_LEN;
	tcpHeader = (struct ntTCPHeader*)dataBuffer;
	//Fill UDP header
	tcpHeader->srcPort = HTONS(pSession->srcPort);
	tcpHeader->desPort = HTONS(pSession->desPort);
	tcpHeader->seqNumber = HTONL(pSession->seqNumber);
	pSession->seqNumber = pSession->seqNumber + len;
	if(Flags & (TCP_FIN_FLAG|TCP_SYN_FLAG))
		(pSession->seqNumber)++;
	tcpHeader->ackNumber = HTONL(pSession->ackNumber);
	if(Flags & TCP_SYN_FLAG){
		tcpHeader->Offset = (0x07<<4);
		len += (TCP_HEADER_LEN + 8);
	}else{
		tcpHeader->Offset = (0x05<<4);
		len += TCP_HEADER_LEN;
	}
	tcpHeader->Flags = Flags;
	tcpHeader->Window = HTONS(pSession->srcWin);//((NETSTACK_BUFFERSIZE-20-14));
	tcpHeader->Checksum = 0;
	tcpHeader->UrgentPtr = 0x0000;
	//Generate checksum
	ipHeader = (struct ntIPHeader*)(dataBuffer-IP_HEADER_LEN);
	ipHeader->srcIPAddr = HTONL(ipGetConfig()->ip);
	ipHeader->desIPAddr = HTONL(pSession->desIP);
	ipHeader->Checksum = HTONS(len);
	ipHeader->TTL = 0x00;
	ipHeader->Protocol = IP_PROTO_TCP;
	checksum32 = 0;
	tmp = len + 12;
	tmp = checksum (((unsigned char *)ipHeader+8), tmp, checksum32);
	tcpHeader->Checksum = HTONS(tmp);
	ipSend(pSession->desIP, IP_PROTO_TCP, len, (unsigned char *)tcpHeader);	
}
//--------------------------------------------------------------------------------------
//Ham xu ly goi TCP nhan duoc, duoc goi boi giao thuc IP (IPProcess)
void TCPProcess(unsigned char *buffer, unsigned int len)
//Ham xu ly cho giao thuc TCP
// Duoc thuc thi khi nhan duoc mot goi TCP (goi boi netstackIPProcess)
// buffer: co tro den dau goi IP (bat dau IP Header)
// len   : chieu dai buffer
{
	unsigned char i,ipHeaderLen,tcpHeaderLen;
	unsigned int dataLen;
	unsigned long tmp;
	struct ntIPHeader* ipHeader;
	struct ntTCPHeader* tcpHeader;
	unsigned char *tcpData;
	//Khoi tao cac co tro den Header IP va TCP
	ipHeader = (struct ntIPHeader*)(buffer);
	ipHeaderLen = ((ipHeader->verHdrLen) & 0x0F) << 2;
	//
	tcpHeader = (struct ntTCPHeader*)(buffer+ipHeaderLen);
	tcpHeaderLen = ((tcpHeader->Offset) & 0xF0) >> 2;
	//
	tcpData = (buffer+ipHeaderLen+tcpHeaderLen);
	dataLen = HTONS(ipHeader->Len) - (ipHeaderLen + tcpHeaderLen);
	//Tim kiem mot phien TCP co san cho goi nay
	for(i = 0; i < TCP_MAX_SESSION; i++){	//Check session table
		if(tcpSessionTable[i].sesState != TCP_STATE_CLOSED){		//If not closed session
			if(tcpSessionTable[i].srcPort == HTONS((tcpHeader->desPort))){	//If matched local port
				if(tcpSessionTable[i].desPort == HTONS((tcpHeader->srcPort))&&(tcpSessionTable[i].desIP == HTONL((ipHeader->srcIPAddr)))){
					break;	//Thoat khoi vong lap for, luc nay gia tri cua i chinh la chi so cua phien TCP tuong ung
				}
			}
		}
	}
	if(i == TCP_MAX_SESSION){	//Neu khong co 1 phien TCP dang ton tai cho goi nay
		//Tim 1 phien dang o trang thai LISTEN (doi ket noi) cho local port nay
		for(i=0; i < TCP_MAX_SESSION; i++){
			if(tcpSessionTable[i].sesState == TCP_STATE_LISTEN){
				if(tcpSessionTable[i].srcPort == HTONS((tcpHeader->desPort))){	//If matched local port
					//Cap nhat remote port va remote IP
					tcpSessionTable[i].desPort = HTONS((tcpHeader->srcPort));
					tcpSessionTable[i].desIP = HTONL((ipHeader->srcIPAddr));
					//Dong thoi tao ra 1 session moi de cho ket noi khac den local port nay
					TCPCreateSession(tcpSessionTable[i].srcPort,tcpSessionTable[i].appDataIn);
					break;
				}
			}
		}
	}
	if(i == TCP_MAX_SESSION){
		#ifdef TCP_DEBUG
		printf("No TCP session found\r\n");
		#endif
		return;	//Neu khong co phien TCP nao danh cho goi nay thi thoat ra
	}
	#ifdef TCP_DEBUG
	printf("TCP session found: %d\r\n",i);
	#endif
	//Bat dau xu ly giao thuc
	tcpSessionTable[i].timeOut = TCP_TIMEOUT;	//Reset lai gia tri Time out
	//Truong hop nhan duoc yeu cau reset lai ket noi
	if ((tcpHeader->Flags) & TCP_RST_FLAG){
		//Chap nhan dong ket noi
		TCPCloseSession(i);
		return;
	}
	//Kiem tra trang thai hien tai cua phien TCP
	switch (tcpSessionTable[i].sesState){
		//Neu la trang thai doi ket noi: TCP_STATE_LISTEN
		case(TCP_STATE_LISTEN):
			//Chi xu ly neu co SYN duoc set (yeu cau thiet lap ket noi)
			if ((tcpHeader->Flags) == TCP_SYN_FLAG){
				//Chuyen sang trang thai ke tiep la TCP_STATE_SYN_RECEIVED
				tcpSessionTable[i].sesState = TCP_STATE_SYN_RECEIVED;
				//Khoi tao gia tri sequence
				tcpSessionTable[i].seqNumber = HTONL(TCPInitSequenceNumber());
				//Ack chinh la so tuan tu nhan duoc cong 1
				tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber))+1;
				tcpSessionTable[i].desWin = HTONS((tcpHeader->Window));
				//Goi tra xac nhan va co SYN (SYN & ACK)
				TCPPackedSend(&tcpSessionTable[i],(TCP_SYN_FLAG|TCP_ACK_FLAG),0,tcpData);
				//Tang so tuan tu len 1
				//tcpSessionTable[i].seqNumber++;
				#ifdef TCP_DEBUG
				printf("SYN received\r\n");
				#endif
			}
			break;
		//Neu la trang thai TCP_STATE_SYN_RECEIVED
		case(TCP_STATE_SYN_RECEIVED):
			//Neu co co ACK (cho ban tin SYN & ACK truoc do)
			if ((tcpHeader->Flags) == TCP_ACK_FLAG){
				//Kiem tra ack trong goi tin den, neu dung thi thiet lap ket noi hoan tat
				if((tcpSessionTable[i].seqNumber) == HTONL((tcpHeader->ackNumber))){
					tcpSessionTable[i].sesState = TCP_STATE_ESTABLISHED;
					//Goi tiep theo gui di se co co ACK
					tcpSessionTable[i].nextAck = 1;
					#ifdef TCP_DEBUG
					printf("Connection established\r\n");
					#endif
				}
			}else{	//Neu khong dung ACK
				//Khong lam gi ca, goi tin do khong hop le
				//TCPCloseSession(i);
			}
			break;
		//Truong hop ket noi da duoc thiet lap
		case(TCP_STATE_ESTABLISHED):
			//Neu nhan duoc yeu cau ket thuc ket noi tu client
			if ((tcpHeader->Flags) & TCP_FIN_FLAG){
				//Chuyen sang trang thai ke tiep la trang thai cho ACK cuoi
				//Dung ra o day phai chuyen sang trang thai TCP_STATE_CLOSE_WAIT nhung khong can thiet
				//  vi o day ta co the dong ket noi ngay ma khong can cho gui xong du lieu
				tcpSessionTable[i].sesState = TCP_STATE_LAST_ACK;
				//Cap nhat ack
				tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber)) + dataLen;
				tcpSessionTable[i].ackNumber++;	//Tang 1 cho co FIN
				//Gui xac nhan ACK cho yeu cau dong ket noi dong thoi thong bao san sang dong ket noi
				TCPPackedSend(&tcpSessionTable[i],TCP_ACK_FLAG,0,tcpData);
				TCPPackedSend(&tcpSessionTable[i],(TCP_FIN_FLAG|TCP_ACK_FLAG),0,tcpData);
				//Dang le truyen o trang thai CLOSE_WAIT nhung ta thuc hien o day luon
				TCPCloseSession(i);
			//Neu khong (dang truyen du lieu)
			}else{
				//Kiem tra ACK tu remote host
				if((tcpHeader->Flags) & TCP_ACK_FLAG){	//Neu co co ACK thi kiem tra gia tri ACK
					tcpSessionTable[i].lastRxAck = HTONL((tcpHeader->ackNumber));
					if ((tcpSessionTable[i].seqNumber) == HTONL((tcpHeader->ackNumber))){	//Dung ACK
						#ifdef TCP_DEBUG
						printf("Got ACK\r\n");
						#endif
					}else{	//Phia ben kia khong nhan duoc du thong tin
						//Sua loi o day
						//Process error correction here
						//Not finish yet, temporary just ignore it and continue with next data
						//Chua thuc hien
						tcpSessionTable[i].seqNumber = HTONL((tcpHeader->ackNumber));
						#ifdef TCP_DEBUG
						printf("Miss ACK:got %d\r\nExpected:%d\n\r",HTONL((tcpHeader->ackNumber)),tcpSessionTable[i].seqNumber+1);										
						#endif
					}
				}
				//--Ket thuc kiem tra ACK
				//Kiem tra sequence number
				tmp = HTONL((tcpHeader->seqNumber));
				//Neu khong dung goi dang cho nhan
				if (tmp != tcpSessionTable[i].ackNumber){
					//there was an error, check what to do next:
					#ifdef TCP_DEBUG
					printf("Incorrect seq, got:%d,expexted:%d\r\n",tmp,tcpSessionTable[i].ackNumber);
					#endif
					if (tmp < tcpSessionTable[i].ackNumber){
						//Neu dang doi du lieu bat dau tu byte thu n nhung ta nhan duoc doan du lieu bat dau tu (n-k)
						//Tinh phan du lieu thua (k = n - (n-k))
						tmp = (tcpSessionTable[i].ackNumber - tmp);
						//Neu doan du lieu thua it hon du lieu nhan duoc
						if(tmp < dataLen){
							//Bo di phan du lieu thua, nhan phan con lai
							tcpData += tmp;
							dataLen = dataLen - tmp;
						}else{	//Neu tat ca du lieu nhan duoc deu thua
							//Gui lai ACK, bo goi vua nhan duoc
							dataLen = 0;
							TCPPackedSend(&tcpSessionTable[i],(TCP_ACK_FLAG),0,tcpData);
							return;
						}
					//Neu seq > ack (tuc la co 1 doan du lieu bi mat)
					}else{ //tmp > tcp....
						//Yeu cau gui lai
						TCPPackedSend(&tcpSessionTable[i],(TCP_ACK_FLAG),0,tcpData);
						return;
					}
				}
				//Neu thuc thi den day nghia la sequence number == ack number (chinh xac)
				//--Ket thuc kiem tra so tuan tu
				//Kiem tra chieu dai buffer de chac chan la chieu dai du lieu nhan duoc khong qua buffer
				//
				if (tcpData > (buffer + ETHERNET_BUFFER_SIZE))
					tcpData = (buffer + ETHERNET_BUFFER_SIZE);
				if ((tcpData + dataLen) > buffer + ETHERNET_BUFFER_SIZE){
					dataLen = (buffer + ETHERNET_BUFFER_SIZE) - tcpData;				
				}
				//
				//Cap nhat ack cho lan nhan ke tiep
				tcpSessionTable[i].ackNumber = tcpSessionTable[i].ackNumber + dataLen;
				#ifdef TCP_DEBUG
				printf("Data length (%d), buffer size(%d)\n\r",dataLen,(buffer + ETHERNET_BUFFER_SIZE - tcpData));
				printf("Ack Number (%d)\n\r",tcpSessionTable[i].ackNumber);
				#endif
				//Goi tiep theo gui di se co co ACK
				tcpSessionTable[i].nextAck = 1;
				//Goi ham xu ly lop ung dung
				if(dataLen != 0){
					(tcpSessionTable[i].appDataIn)(tcpData, dataLen,&tcpSessionTable[i]);
				}
			}
			//--Ket thuc xu ly truong hop dang truyen du lieu
			break;
		//Neu la trang thai doi LAST_ACK (2 phia deu san sang dong ket noi, dang doi xac nhan ack cuoi cung)
		case(TCP_STATE_LAST_ACK):
			//socket is closed
			tmp = HTONL((tcpHeader->seqNumber));
			//Kiem tra ACK, neu dung ACK
			if (tmp == tcpSessionTable[i].seqNumber + 1){
				TCPCloseSession(i);
			}else{
				//Gui lai co FIN & ACK
				TCPPackedSend(&tcpSessionTable[i], (TCP_FIN_FLAG|TCP_ACK_FLAG), 0, tcpData);
			}
			break;

		//Truong hop ngat ket noi thu dong, da nhan co FIN tu remote host va xac nhan
		case(TCP_STATE_CLOSE_WAIT):
			//Truong hop nay se khong xay ra vi o tren ta chuyen truc tiep
			//  sang LAST_ACK khi nhan duoc yeu cau dong ket noi
			tcpSessionTable[i].sesState = TCP_STATE_LAST_ACK;
			if(dataLen){
				tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber)) + dataLen;
			}else{	//Neu dataLen == 0 thi cung tang so tuan tu len 1
				tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber))+1;
			}
			//tcpSessionTable[i].seqNumber = HTONL((tcpHeader->ackNumber));
			TCPPackedSend(&tcpSessionTable[i], (TCP_FIN_FLAG|TCP_ACK_FLAG), 0, tcpData);
			break;
		//Truong hop dang o trang thai FIN WAIT 1 (da truyen du lieu xong,
		//  san sang dong ket noi va da gui di co FIN va dang cho ACK)
		case(TCP_STATE_FIN_WAIT1):
			//if we receive FIN
			tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber))+1;
			if (tcpHeader->Flags == TCP_FIN_FLAG){	//Neu chi nhan duoc co FIN
				//Chuyen sang trang thai CLOSING va gui ACK
				tcpSessionTable[i].sesState = TCP_STATE_CLOSING;
				TCPPackedSend(&tcpSessionTable[i], (TCP_ACK_FLAG), 0, tcpData);
				//tcpSessionTable[i].seqNumber++;
				#ifdef TCP_DEBUG
				printf("Closing\n\r");
				#endif
			}else if(tcpHeader->Flags == (TCP_FIN_FLAG | TCP_ACK_FLAG)){	//Neu nhan dong thoi FIN va ACK
				//Chuyen sang trang thai TIME_WAIT va gui ACK
				//  nhung o day do chua co timer nen ta chuyen luon sang dong ket noi
				if (HTONL((tcpHeader->ackNumber)) == tcpSessionTable[i].seqNumber){
					//TCPPackedSend(&tcpSessionTable[i], (TCP_ACK_FLAG), 0, tcpData);
					TCPCloseSession(i);
					#ifdef TCP_DEBUG
					printf("End\n\r");
					#endif
				}else{	//Neu khong dung ack cho thong bao FIN
					//Chuyen sang cho co ACK cuoi cung
					tcpSessionTable[i].sesState = TCP_STATE_LAST_ACK;
					#ifdef TCP_DEBUG
					printf("Last ack\n\r");
					#endif
				}
				//Gui xac nhan cho co FIN
				TCPPackedSend(&tcpSessionTable[i], (TCP_ACK_FLAG), 0, tcpData);
				//tcpSessionTable[i].seqNumber++;
			}else if(tcpHeader->Flags == TCP_ACK_FLAG){	//Neu chi nhan duoc ACK
				//Chuyen sang trang thai FIN WAIT2
				tcpSessionTable[i].sesState = TCP_STATE_FIN_WAIT2;
				#ifdef TCP_DEBUG
				printf("Fin wait 2\n\r");
				#endif
			}
			break;
		//Neu dang o trang thai FIN WAIT 2 (san sang dong ket noi va gui co FIN,
		//  phia ben kia da xac nhan nhung van chua san sang dong ket noi
		case(TCP_STATE_FIN_WAIT2):
			//Neu nhan duoc co FIN
			if (tcpHeader->Flags & TCP_FIN_FLAG){
				if(dataLen){
					tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber))+dataLen;
				}else{
					tcpSessionTable[i].ackNumber = HTONL((tcpHeader->seqNumber))+1;
				}
				//FIN -> goto TIMED WAIT
				tcpSessionTable[i].sesState = TCP_STATE_TIMED_WAIT;
				TCPPackedSend(&tcpSessionTable[i], (TCP_ACK_FLAG), 0, tcpData);
				//Chua co timer thi dong ket noi o day luon
				TCPCloseSession(i);
				#ifdef TCP_DEBUG
				printf("End\n\r");
				#endif
			}
			break;
		case(TCP_STATE_TIMED_WAIT):
			break;
		case(TCP_STATE_CLOSING):
			tcpSessionTable[i].sesState = TCP_STATE_TIMED_WAIT;
			break;
		default:
			TCPCloseSession(i);
	}
	//we must set timed wait TTL here because timed wait is not packet triggered
	if (tcpSessionTable[i].sesState == TCP_STATE_TIMED_WAIT){
		tcpSessionTable[i].timeOut = 5; //5 seconds timeout
	}
	return;
}
//--------------------------------------------------------------------------------------
