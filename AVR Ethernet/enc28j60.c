/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include <avr/io.h>
#include "AVRnet.h"
#include "enc28j60.h"
#include "enc28j60conf.h"
#include <avr/pgmspace.h>

unsigned char Enc28j60Bank;

unsigned char enc28j60SPIRead(unsigned char op, unsigned char address)
{
	unsigned char res;
	ENC28J60_CS_LO();
	SPDR = op | (address & ADDR_MASK);
	while(!(SPSR & (1<<SPIF))); 
	SPDR = 0x00; 
	while(!(SPSR & (1<<SPIF))); 
	if(address & 0x80){  
		SPDR = 0x00;
		while(!((SPSR) & (1<<SPIF)));
	}
	res = SPDR; 
	ENC28J60_CS_HI();
	return res;
}

void enc28j60SPIWrite(unsigned char op, unsigned char address, unsigned char data)
{
	ENC28J60_CS_LO();
	SPDR = op | (address & ADDR_MASK);
	while(!(SPSR & (1<<SPIF)));
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	ENC28J60_CS_HI();
}

void enc28j60SetBank(unsigned char address)
{
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		enc28j60SPIWrite(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60SPIWrite(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}

void enc28j60ReadBuffer(unsigned int len, unsigned char* data)
{
	ENC28J60_CS_LO();
	SPDR = ENC28J60_READ_BUF_MEM;
	while(!(SPSR & (1<<SPIF)));
	while(len--)
	{
		SPDR = 0x00;
		while(!(SPSR & (1<<SPIF)));
		*data++ = SPDR;
	}	
	ENC28J60_CS_HI();
}

void enc28j60WriteBuffer(unsigned int len, unsigned char* data)
{
	ENC28J60_CS_LO();
	SPDR = ENC28J60_WRITE_BUF_MEM;
	while(!(SPSR & (1<<SPIF)));
	while(len--)
	{
		SPDR = *data++;
		while(!(SPSR & (1<<SPIF)));
	}	
	ENC28J60_CS_HI();
}

unsigned char enc28j60Read(unsigned char address)
{
	enc28j60SetBank(address);
	return enc28j60SPIRead(ENC28J60_READ_CTRL_REG, address);
}

void enc28j60Write(unsigned char address, unsigned char data)
{
	enc28j60SetBank(address);
	enc28j60SPIWrite(ENC28J60_WRITE_CTRL_REG, address, data);
}

unsigned int enc28j60PhyRead(unsigned char address)
{
	unsigned int data;
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MICMD, MICMD_MIIRD);
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
	enc28j60Write(MICMD, 0x00);
	data  = enc28j60Read(MIRDL);
	data |= enc28j60Read(MIRDH);
	return data;
}

void enc28j60PhyWrite(unsigned char address, unsigned int data)
{
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MIWRL, data);	
	enc28j60Write(MIWRH, data>>8);
	while(enc28j60Read(MISTAT) & MISTAT_BUSY);
}

void enc28j60PacketSend(unsigned int len, unsigned char* packet)
{
	enc28j60Write(EWRPTL, TXSTART_INIT);
	enc28j60Write(EWRPTH, TXSTART_INIT>>8);
	enc28j60Write(ETXNDL, (TXSTART_INIT+len));
	enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);
	enc28j60SPIWrite(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

	enc28j60WriteBuffer(len, packet);
	enc28j60SPIWrite(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

unsigned int NextPacketPtr;

unsigned int enc28j60PacketReceive(unsigned int maxlen, unsigned char* packet)
{
	unsigned int rxstat;
	unsigned int len;
	if( !enc28j60Read(EPKTCNT) )
		return 0;
	enc28j60Write(ERDPTL, (NextPacketPtr));
	enc28j60Write(ERDPTH, (NextPacketPtr)>>8);
	NextPacketPtr  = enc28j60SPIRead(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= ((unsigned int)enc28j60SPIRead(ENC28J60_READ_BUF_MEM, 0))<<8;
	len  = enc28j60SPIRead(ENC28J60_READ_BUF_MEM, 0);
	len |= ((unsigned int)enc28j60SPIRead(ENC28J60_READ_BUF_MEM, 0))<<8;
	rxstat  = enc28j60SPIRead(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= ((unsigned int)enc28j60SPIRead(ENC28J60_READ_BUF_MEM, 0))<<8;
	len = ((len<maxlen)?(len):(maxlen));
	enc28j60ReadBuffer(len, packet);
	enc28j60Write(ERXRDPTL, (NextPacketPtr));
	enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);
	enc28j60SPIWrite(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

	return len;
}

prog_char enc28j60_config[44] PROGMEM = {
	ETXSTL, LO8(TXSTART_INIT), //start lo
	ETXSTH, HI8(TXSTART_INIT), //start hi
	ETXNDL, LO8(TXSTOP_INIT  ), //end lo
	ETXNDH, HI8(TXSTOP_INIT  ), //end hi

	ERXSTL, LO8(RXSTART_INIT), //start lo
	ERXSTH, HI8(RXSTART_INIT), //start hi
	ERXNDL, LO8(RXSTOP_INIT  ), //end lo
	ERXNDH, HI8(RXSTOP_INIT  ), //end hi

	MACON2, 	0x00,
	MACON1, 	(MACON1_MARXEN | MACON1_RXPAUS | MACON1_TXPAUS),
	MACON3, 	( MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN),
	MAMXFLL, LO8(1518),
	MAMXFLH, HI8(1518),
	MABBIPG, 0x12, //half duplex
	MAIPGL,  0x12,
	MAIPGH,  0x0C, //half duplex

	MAADR5, ENC28J60_MAC0,
	MAADR4, ENC28J60_MAC1,
	MAADR3, ENC28J60_MAC2,
	MAADR2, ENC28J60_MAC3,
	MAADR1, ENC28J60_MAC4,
	MAADR0, ENC28J60_MAC5
	};

void delay_us(unsigned short time_us) 
{
	unsigned short delay_loops;
	register unsigned short i;

	delay_loops = (time_us+3)/5*CYCLES_PER_US; // +3 for rounding up (dirty) 

	// one loop takes 5 cpu cycles 
	for (i=0; i < delay_loops; i++) {};
}

void enc28j60Init(void)
{
	unsigned char i;
	unsigned int timeout=0;
	Enc28j60Bank = 0xFF;
	ENC28J60_CONTROL_DDR |= (1<<ENC28J60_CONTROL_CS);
	ENC28J60_CS_HI();
	ENC28J60_SPI_PORT |= (1<<ENC28J60_SPI_SCK); //sck = hi
	ENC28J60_SPI_DDR |= (1<<ENC28J60_SPI_SS)|(1<<ENC28J60_SPI_MOSI)|(1<<ENC28J60_SPI_SCK); //SS,MOSI,SCK = OUT
	ENC28J60_SPI_DDR &= ~(1<<ENC28J60_SPI_MISO); //MISO = IN
	SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
//	SPSR = (1<<SPI2X);
	SPSR = (0<<SPI2X);
	delay_us(65000);delay_us(65000);delay_us(65000);
	enc28j60SPIWrite(ENC28J60_SOFT_RESET,0, ENC28J60_SOFT_RESET);
	delay_us(65000);delay_us(65000);delay_us(65000);
	while((!(enc28j60Read(ESTAT) & 0x01)) && (timeout<65000)){timeout++;};
	if(timeout>=65000){timeout=0;}
	NextPacketPtr = RXSTART_INIT;
	for(i=0; i<2*22; i+=2){
		enc28j60Write(pgm_read_byte(&enc28j60_config[i+0]),pgm_read_byte(&enc28j60_config[i+1]));
	}
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS); //=no loopback of transmitted frames
	enc28j60SPIWrite(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	enc28j60SPIWrite(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
	enc28j60PhyWrite(PHLCON, 0x347A); //cave: Table3-3: reset value is 0x3422, do not modify the reserved "3"!! 
}

