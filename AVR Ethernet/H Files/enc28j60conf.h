//----------------------------------------------------------------------------
// Writen by NTTam - PTITHCM
//----------------------------------------------------------------------------
#ifndef ENC28J60CONF_H
#define ENC28J60CONF_H
//
	//Khai bao cac chan IO cho ENC28J60
	#define ENC28J60_CONTROL_DDR	DDRB
	#define ENC28J60_CONTROL_PORT	PORTB
	#define ENC28J60_SPI_DDR		DDRB
	#define ENC28J60_SPI_PORT		PORTB
	//
	#define ENC28J60_CONTROL_CS		3
	#define ENC28J60_CONTROL_RESET	4
	#define ENC28J60_SPI_SCK		7
	#define ENC28J60_SPI_MISO		6
	#define ENC28J60_SPI_MOSI		5
	#define ENC28J60_SPI_SS			4
	#define ENC28J60_SPI_CS			3
	//
	//Dinh nghia macro chon chip ENC28J60
	#define ENC28J60_CS_LO()    ENC28J60_CONTROL_PORT &= ~(1<<ENC28J60_CONTROL_CS);
	#define ENC28J60_CS_HI()    ENC28J60_CONTROL_PORT |=  (1<<ENC28J60_CONTROL_CS);
	//
	#define ETH_INTERRUPT 		INT2_vect
	//
	#if defined (__AVR_ATmega32__)
		#define ETH_INT_ENABLE 	GICR |= (1<<INT2)
		#define ETH_INT_DISABLE GICR &= ~(1<<INT2)
	#endif

	#if defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__)
		#define ETH_INT_ENABLE 	EIMSK |= (1<<INT2)
		#define ETH_INT_DISABLE EIMSK &= ~(1<<INT2)
	#endif

// MAC address for this interface
	#ifdef ETHADDR0
	#define ENC28J60_MAC0 ETHADDR0
	#define ENC28J60_MAC1 ETHADDR1
	#define ENC28J60_MAC2 ETHADDR2
	#define ENC28J60_MAC3 ETHADDR3
	#define ENC28J60_MAC4 ETHADDR4
	#define ENC28J60_MAC5 ETHADDR5
	#else
	#define ENC28J60_MAC0 '0'
	#define ENC28J60_MAC1 'F'
	#define ENC28J60_MAC2 'F'
	#define ENC28J60_MAC3 'I'
	#define ENC28J60_MAC4 'C'
	#define ENC28J60_MAC5 'E'
	#endif


#endif // ENC28J60CONF_H
//----------------------------------------------------------------------------
