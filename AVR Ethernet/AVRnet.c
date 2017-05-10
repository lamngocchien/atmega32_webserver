/*
 * AVR ETHERNET
 *
 * Created: 11/28/2012 9:34:10 PM
 *  Author: Lam Ngoc Chien
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include "ethernet.h"
#include "ip.h"
#include "arp.h"
#include "timer.h"
#include "uart.h"
#include "dhcp.h"
#include "tcp.h"
#include "http.h"
#include "AVRnet.h"
#include "lcd.h"
#include "rtc.h"
#include "1wire.h"
extern struct ipConfig IpMyConfig;
//Cac bien RTC
volatile uint8_t	Second=30, Minute=16, Hour=20, Day=2, Date=15, Month=4, Year=13, Mode=0, AP=1;
volatile  uint8_t tData[7];
char dis[5];
//Cac bien DS18B20
uint8_t ds18b20, digit;

//Cac ham DS18B20
unsigned char bao_dong, ngung_bao_dong;
void alarm()
{	
	if(ds18b20 >= ALARM)	{bao_dong++;	ngung_bao_dong=0;}
	else{ngung_bao_dong++;	bao_dong = 0;}
	if(bao_dong == 2)	{ON_ALARM	bao_dong = 0;}
	else	if(ngung_bao_dong >= 5)	{OFF_ALARM	ngung_bao_dong = 0;}			
}
void showtemp ()
{
	move_LCD(1,10);
	therm_read_temperature();
	lcd_show_number(ds18b20);
	print_LCD(".");
	putChar_LCD(digit/10 +48);
	putChar_LCD(digit%10 +48);
	putChar_LCD(0xdf);	print_LCD("C");	print_LCD(" ");
	check_auto();
	alarm();
}
//Cac ham theo RTC
uint8_t BCD2Dec(uint8_t BCD){
	uint8_t L, H;
	L=BCD & 0x0F;
	H=(BCD>>4)*10;
	return (H+L);
}
uint8_t Dec2BCD(uint8_t Dec){
	uint8_t L, H;
	L=Dec % 10;
	H=(Dec/10)<<4;
	return (H+L);
}
void	Display	()
{ 
	Second 	= BCD2Dec(tData[0] & 0x7F);
	Minute 	= BCD2Dec(tData[1]);
	
	if (Mode !=0) 	Hour = BCD2Dec(tData[2] & 0x1F); //mode 12h
	else 		  	Hour = BCD2Dec(tData[2] & 0x3F); //mode 24h	
	Day		= BCD2Dec(tData[3]);
	Date   	= BCD2Dec(tData[4]);
	Month	= BCD2Dec(tData[5]);
	Year	= BCD2Dec(tData[6]);
	
	clr_LCD();		//xoa LCD
	//Xuat gio:phut:giay
	//Xuat Hour
	sprintf(dis, "%i",Hour);
	move_LCD(1,1);  if (Hour < 10) putChar_LCD(' ');print_LCD(dis); 
	move_LCD(1,3); putChar_LCD(':');
	//Xuat Minute
	sprintf(dis, "%i",Minute); 
	move_LCD(1,4); if (Minute < 10) putChar_LCD('0');		print_LCD(dis); 
	move_LCD(1,6);	putChar_LCD(':');
	//Xuat Second
	sprintf(dis, "%i",Second); 
	move_LCD(1,7); if (Second<10) putChar_LCD('0');		print_LCD(dis); 
	if (Mode != 0)
	{ 
		//mode 12h
		move_LCD(1,1);
		if (bit_is_set(tData[2],5))  putChar_LCD('P'); //kiem tra bit AP, if AP=1
		else putChar_LCD('A');
	}
	//Xuat nam-thang-ngay
	move_LCD(2,1);
	switch (Day)
		{
		case 1:		print_LCD("Sun");	break;
		case 2:		print_LCD("Mon");	break;
		case 3:		print_LCD("Tue");	break;
		case 4:		print_LCD("Wed");	break;
		case 5:		print_LCD("Thu");	break;
		case 6:		print_LCD("Fri");	break;
		case 7:		print_LCD("Sat");	break;
		}
	
	//Xuat Date
	sprintf(dis, "%i",Date);  
	move_LCD(2,5); 
	if (Date<10)	putChar_LCD(' '); 
	print_LCD(dis);
	move_LCD(2,7);	putChar_LCD('/');//dau cach 1
	
	//Xuat Month
	sprintf(dis, "%i",Month);  
	if (Month<10)	putChar_LCD('0'); 
	print_LCD(dis); 
	move_LCD(2,10); putChar_LCD('/');//dau cach 2

	//Xuat Year
	putChar_LCD('2'); putChar_LCD('0');//xuat so '20'
	sprintf(dis, "%i",Year);	
	move_LCD(2,13);
	if (Year < 10) putChar_LCD('0'); // neu nam <10, in them so 0 ben trai, vi du 09 
	print_LCD(dis);	
}
void set_clock()
{
	unsigned char temp, flag = 1;// mac dinh la cho phep ghi gia tri thoi gian vao ds1307
	//Kiem tra first time qua byte 0x00 trong eeprom da duoc ghi gia tri bao gio chua
	while(!eeprom_is_ready());
	temp = eeprom_read_byte(0); // gia tri mac dinh ban dau cua byte 0 trong eeprom la 0xff = 255
	_delay_ms(1);
	if(temp != 240)
		{
			while(!eeprom_is_ready());
			eeprom_write_byte(0,240); // danh dau lan ghi dau tien 0xf0
			_delay_ms(1);
		}
	else
			flag = 0;
	//Ghi tri ghi vao DS1307
	if(flag)
	{
		//must_edit = 0; // Su dung de tinh chinh thoi gian sau nay
		tData[0] = Dec2BCD(Second); 
		tData[1] = Dec2BCD(Minute); 
		if (Mode != 0) 
			tData[2] = Dec2BCD(Hour)|(Mode<<6)|(AP<<5); //mode 12h
		else 
			tData[2] = Dec2BCD(Hour);
		tData[3] = Dec2BCD(Day);
		tData[4] = Dec2BCD(Date);
		tData[5] = Dec2BCD(Month); 
		tData[6] = Dec2BCD(Year); 		
		TWI_Init(); //khoi dong TWI		
		TWI_DS1307_wblock(0x00, tData, 7); //ghi lien tiep cac bien thoi gian vao DS1307
		_delay_ms(1);	//cho DS1307 xu li 
	}
}
void showclock()
{
	TWI_DS1307_wadr(0x00); 				//set dia chi ve 0
	_delay_ms(1);		   				//cho DS1307 xu li 
	TWI_DS1307_rblock(tData,1); 	//doc ca khoi thoi gian (7 bytes)		
	//hien thi ket qua len LCD
	if(BCD2Dec(tData[0]) !=Second)
	{ 	
		//chi hien thi ket qua khi da qua 1s
		Second=BCD2Dec(tData[0] & 0x7F);
		sprintf(dis, "%i",Second); 
		move_LCD(1,7); 
		if (Second < 10)	putChar_LCD('0');	print_LCD(dis);
		if (Second == 0) 
		{
			TWI_DS1307_wadr(0x00); 				//set dia chi ve 0
			_delay_ms(1);		   				//cho DS1307 xu li 
			TWI_DS1307_rblock(tData,7); 	//doc ca khoi thoi gian (7 bytes)	
			Display(); 		//moi phut cap nhat 1 lan		
		}
		if(Second == 1)	active_countdown();	
		if(Second %5 == 0) showip();
		showtemp();		
	}	
}

//Cac ham set Auto
uint16_t countdown_min[5];
uint8_t  state_count[5];
uint8_t auto_off[5], auto_state[5];
void edit_day(uint8_t day)
{
	//cli();
	tData[3] = Dec2BCD(day);
	TWI_Init();	
	TWI_DS1307_wblock(0x00, tData, 7);
	_delay_ms(1);	
	//Xuat lai gia tri time
	TWI_DS1307_wadr(0x00);
	_delay_ms(1);
	TWI_DS1307_rblock(tData,7);
	Display();
	//sei();
}
void edit_clock(uint8_t type, uint8_t val1, uint8_t val2, uint8_t val3)
{
	//cli();
	uint16_t new_count[5];
	uint8_t new_state[5];
	uint8_t copy_tData[7];
	//Case HMS:  Thay doi gio phut giay
	if(type == 1)
	{
		tData[0] = Dec2BCD(val3); //Giay
		tData[1] = Dec2BCD(val2); //Phut
		tData[2] = Dec2BCD(val1); //Gio
	}
	//Case DMY:  Thay doi ngay thang nam
	else
	if(type == 2)
	{
		tData[4] = Dec2BCD(val1); //Ngay
		tData[5] = Dec2BCD(val2); //Thang
		tData[6] = Dec2BCD(val3); //Nam		
	}
	TWI_Init();	
	TWI_DS1307_wblock(0x00, tData, 7);
	_delay_ms(1);	
	//Xuat lai gia tri time
	TWI_DS1307_wadr(0x00); //set dia chi ve 0
	_delay_ms(1);			//cho DS1307 xu li 
	TWI_DS1307_rblock(tData,7); //doc ca khoi thoi gian (7 bytes)	
	Display(); // hien thi ket qua len LCD		
	uint8_t flag = 0;
	for(uint8_t i = 1; i<4; i++)
		if(state_count[i] == 15 || state_count[i] == 240)
			{	flag = 1;	break;}
	if(flag)
		{
		//Update EEPROM: Cac gia tri lien quan den hen gio
		for(uint8_t i = 1; i<=4; i++)
			if(state_count[i] == 240 || state_count[i] == 15)
				{	new_count[i] = countdown_min[i];	new_state[i] = state_count[i]; }
			else
				{	new_count[i] = 0xffff;	new_state[i] = 0xff; }	
	
		//Ghi thoi gian vao eeprom vi tri tu 1 - 6, moi lan 1 byte	
		copy_tData[2] = BCD2Dec(tData[2] & 0x3F);
		for (uint8_t i = 1; i <= 6; i++)
			{
			if(i != 2)
				copy_tData[i] = BCD2Dec(tData[i]);
			while(!eeprom_is_ready());
			eeprom_write_byte(i,copy_tData[i]);
			_delay_ms(1);
			}
	
		//Ghi count vao eeprom vi tri tu 7 - 14, moi lan 2 byte
		for (uint8_t i = 1; i <= 4; i++)
			{
			while(!eeprom_is_ready());
			eeprom_write_word(5+i*2,new_count[i]);
			_delay_ms(1);
			while(!eeprom_is_ready());
			eeprom_write_byte(14+i,new_state[i]);
			_delay_ms(1);
			}	
		}
	//sei();
}
void set_time_count(uint8_t device, uint8_t state, uint16_t time)
{
	//Lay moc thoi gian moi
	uint8_t new_clock[7];
	new_clock[2] = BCD2Dec(tData[2] & 0x3F);
	for(uint8_t i = 1; i <= 6; i++)		if(i != 2)	new_clock[i] = BCD2Dec(tData[i]);
	
	//Tao chuoi gia tri moi
	uint16_t new_count[5];
	uint8_t new_state[5];
	countdown_min[device] = new_count[device] = time;
	state_count[device] = new_state[device] = state;
	for(uint8_t i = 1; i <= 4; i++)
		if(i != device)
			if(countdown_min[i] != 0)	{new_count[i] = countdown_min[i];	new_state[i] = state_count[i];}
			else	{new_count[i] = 0xffff;	new_state[i] = 255;}
	//Ghi thoi gian vao eeprom vi tri tu 1 - 6, moi lan 1 byte		
	for (uint8_t i = 1; i <= 6; i++)
		{
			while(!eeprom_is_ready());
			eeprom_write_byte(i,new_clock[i]);
			_delay_ms(1);
		}		
	//Ghi count vao eeprom vi tri tu 7 - 14, moi lan 2 byte
	for (uint8_t i = 1; i <= 4; i++)
		{
			while(!eeprom_is_ready());
			eeprom_write_word(5+i*2,new_count[i]);
			_delay_ms(1);
			while(!eeprom_is_ready());
			eeprom_write_byte(14+i,new_state[i]);
			_delay_ms(1);
		}
}

void set_temp_val(unsigned char temp, unsigned char state, unsigned char device)
{
	begin_auto(device);
	auto_off[device] = temp;
	auto_state[device] = state;	
	while(!eeprom_is_ready());
	eeprom_write_byte(device + 18, temp);
	_delay_ms(1);
	while(!eeprom_is_ready());
	eeprom_write_byte(device + 22, state);
	_delay_ms(1);
}
void active_countdown()
{
	for(uint8_t i = 1;i <= 4;i++)
	if(state_count[i] == 240 || state_count[i] == 15)
		if(countdown_min[i] == 1)
			{
				if(state_count[i] == 240)	on_device(i);
				else	if(state_count[i] == 15)	off_device(i);
				state_count[i] = countdown_min[i] = 0;	
			}							
		else
			if(state_count[i] == 240 || state_count[i] == 15)
				countdown_min[i]--;					
}
uint8_t high[5], low[5];
void begin_auto(uint8_t device)
{
	high[device] = 0;
	low[device] = 0;
}
void check_auto()
{
	//EX: 100ON: ON>100 OFF<100
	//EX: 100OFF: ON<100 OFF>100
	//unsigned char GetState(uint8_t val) // =240 ON =15 OFF
	//Ham Auto ON
	for(uint8_t i = 1; i <= 4; i++)
	{
	if(auto_off[i] != 0 && countdown_min[i] == 0 )
		{	
		if((ds18b20 > auto_off[i]) || (ds18b20 == auto_off[i] && digit > 0))
			{
			high[i]++;
			low[i] = 0;
			if(GetState(i) != auto_state[i] && high[i] >= 10) 
				{
					if (auto_state[i] == 15)	off_device(i);
					else	if (auto_state[i] == 240) on_device(i);
					high[i] = 0;
				}				
			}
		else
		if( ds18b20 < auto_off[i] ||(ds18b20 + digit == auto_off[i]) )
			{
			high[i] = 0;
			low[i]++;				
			if(GetState(i) == auto_state[i] && low[i] >= 10)
				{
					if (auto_state[i] == 15)	on_device(i);
					else	if (auto_state[i] == 240) off_device(i);
					low[i] = 0;
				}				
			}
		}			
	}
}

//Nhom ham backup
void reset_eeprom( uint8_t begin, uint8_t end)
{
	for (unsigned char i = begin; i <= end; i++)
		{
			while(!eeprom_is_ready());	
			eeprom_write_byte(i,255);
			_delay_ms(1);
		}
}
void reset_one_device(uint8_t device)
{	
	countdown_min[device] = state_count[device] = auto_off[device] = auto_state[device] = 0;
	while(!eeprom_is_ready());	eeprom_write_word(5+device*2,255);		_delay_ms(1);
	while(!eeprom_is_ready());	eeprom_write_byte(14+device,255);		_delay_ms(1);
	while(!eeprom_is_ready());	eeprom_write_byte(device+18,255);		_delay_ms(1);
	while(!eeprom_is_ready());	eeprom_write_byte(device+22,255);		_delay_ms(1);
}
void reset_all()
{
	reset_eeprom(1,26);
	for(uint8_t i = 1; i<=4; i++)
		countdown_min[i] = state_count[i] = auto_off[i] = auto_state[i] = 0; 
}
unsigned char max_date(unsigned char val_month, unsigned char val_year)
{
	val_year += 2000;
	if(val_month == 2)	{if(val_year % 4 == 0)	return 29;	else	return 28;}
	else	{if(val_month == 4 || val_month == 6 || val_month == 9 || val_month == 11)	return 30;	else	return 31;}	
}
uint16_t conver2date(unsigned int val_date,unsigned int val_month,unsigned int val_year)
{
	unsigned int kqua;
	kqua = (val_year - 13)*365;	// Tinh so ngay tu nam 2013
	kqua += val_date;
	if(val_month > 1)
		for (uint8_t i = 1;i < val_month;i++)
		{
			kqua += max_date(val_month,val_year);
		}
	if((val_year + 2000) % 4 == 0)
		return kqua++;
	return kqua;
}
uint16_t convert2minute(uint8_t val_min, uint8_t val_hour)
{
	return (val_hour*60 + val_min);
}
void update_auto()
{
	for(uint8_t i = 1; i <= 4; i++)
	{
		while(!eeprom_is_ready());		
		auto_off[i] = eeprom_read_byte(i + 18);
		_delay_ms(1);	
		while(!eeprom_is_ready());
		auto_state[i] = eeprom_read_byte(i + 22);
		_delay_ms(1);	
		if(auto_off[i] == 255)	auto_off[i] = 0;	
	}
}
void update_count()
{
	uint8_t time_eeprom[7],copy_tData[7];
	uint16_t temp_date, temp_min, temp_val, flag=0;
	uint16_t new_count[5];
	uint8_t new_state[5];
	
	//Lay data tu eeprom
	for(uint8_t i = 1; i <= 6; i++)
		{
		while(!eeprom_is_ready());		
		time_eeprom[i] = eeprom_read_byte(i);	
		_delay_ms(1);
		}
	for (uint8_t i = 1; i <= 4; i++)
		{
		while(!eeprom_is_ready());		new_count[i] = eeprom_read_word(5+i*2);		_delay_ms(1);
		while(!eeprom_is_ready());		new_state[i] = eeprom_read_byte(14+i);		_delay_ms(1);
		}
	
	//Kiem tra xem truoc do co hen gio hay khong
	for(uint8_t i = 1;i <= 4; i++)
		if(new_state[i] == 0x0f || new_state[i] == 0xf0)
			{
				flag = 1;	//Co hen gio truoc do				
				break;
			}
	if(flag)
		{
		//Lay thoi gian thuc
		copy_tData[2] = BCD2Dec(tData[2] & 0x3F);
		for(uint8_t i = 1;i <= 6;i++)
			if(i != 2)
				copy_tData[i] = BCD2Dec(tData[i]);

		//Tinh khoang lech thoi gian thuc so voi gia tri trong eeprom
		temp_date = conver2date(copy_tData[4],copy_tData[5],copy_tData[6])-conver2date(time_eeprom[4],time_eeprom[5],time_eeprom[6]);
		temp_min = convert2minute(copy_tData[1],copy_tData[2])-convert2minute(time_eeprom[1],time_eeprom[2]);
		temp_val = temp_date*1440 + temp_min;

		//So sanh thoi gian de xac dinh trang thai can co cua thiet bi dk: khoang lech >0
		if(temp_val > 0)
			for(uint8_t i = 1;i <= 4; i++)
				if(new_count[i] > temp_val &&(new_state[i] == 0x0f || new_state[i] == 0xf0))
					new_count[i]-=temp_val;
				else
					{
						if(new_count[i] <= temp_val)
							if(new_state[i] == 0xf0)
								on_device(i);
							else	if(new_state[i] == 0x0f)
									off_device(i);
						new_count[i] = 0xffff;
						new_state[i] = 0xff;
					}
		//Cap nhat gia tri time
		for(uint8_t i=1;i<=4;i++)
			if(new_state[i] == 240 || new_state[i] == 15)
				{
				countdown_min[i] = new_count[i];
				state_count[i] = new_state[i];						
				}						
		//Ghi thoi gian vao eeprom vi tri tu 1 - 6, moi lan 1 byte	
		for (uint8_t i = 1; i <= 6; i++)
			{
			while(!eeprom_is_ready());
			eeprom_write_byte(i,copy_tData[i]);
			_delay_ms(1);
			}		
		//Ghi count vao eeprom vi tri tu 7 - 14, moi lan 2 byte
		for (uint8_t i = 1; i <= 4; i++)
			{
			while(!eeprom_is_ready());
			eeprom_write_word(5+i*2,new_count[i]);
			_delay_ms(1);
			while(!eeprom_is_ready());
			eeprom_write_byte(14+i,new_state[i]);
			_delay_ms(1);
			}
		}
	else
		if(flag == 0)
			reset_eeprom(1,18);	// Khong co hen gio xoa vung eeprom muc dich hen gio
}
void Backup_System()
{
	update_auto();
	update_count();
}

//Ham DK TB
void off_device (uint8_t val)
{
	switch (val)
		{
			case 1:		OFF1	break;
			case 2: 	OFF2	break;
			case 3: 	OFF3	break;
			case 4: 	OFF4	break;
			default: break;
		}
}
void on_device (uint8_t val)
{
	switch (val)
		{
			case 1:		ON1		break;
			case 2: 	ON2		break;
			case 3: 	ON3		break;
			case 4: 	ON4		break;
			default: break;
		}
}
uint8_t GetState(uint8_t val) // =240 ON =15 OFF
{
	switch (val)
	{
			case 1: 	if(bit_is_clear(PORTC,PINC7))	return 240;	else	return 15;		break;
			case 2: 	if(bit_is_clear(PORTC,PINC6))	return 240;	else	return 15;		break;
			case 3: 	if(bit_is_clear(PORTC,PINC5))	return 240;	else	return 15;		break;
			case 4: 	if(bit_is_clear(PORTC,PINC4))	return 240;	else	return 15;		break;
			default: break;
	}
}
void clrline2lcd(void)
{
	move_LCD(2,1);	for(uint8_t i = 1; i <= 16 ;i++)	putChar_LCD(' ');
}
unsigned char test_out_state(unsigned char device)
{
	if(state_count[device] == 240)
		return 2; // on				
	else	
		if(state_count[device] == 15)
			return 1; //off	
	return 0;		
}

//Xuat 1 so co 3 chu so
void lcd_show_number(uint8_t val)
{
	if(val /100 == 0 && (val %100) /10 != 0)	putChar_LCD((val %100) /10+48);
	else if(val /100 != 0) {putChar_LCD(val /100+48);	putChar_LCD((val %100) /10+48);}
	putChar_LCD((val %100) %10+48);
}
//In ip
void network_out(unsigned long ipaddr, uint8_t kind)
{
	switch(kind)
	{
		case 1: print_LCD("IP:");	break;
		case 2:	print_LCD("SN:");	break;
		case 3:	print_LCD("GW:");	break;
		default: break;
	}
	if(((unsigned char*)&ipaddr)[3] == 0)	{print_LCD("0.0.0.0");}
	else
	{
		lcd_show_number(((unsigned char*)&ipaddr)[3]);	putChar_LCD('.');
		lcd_show_number(((unsigned char*)&ipaddr)[2]);	putChar_LCD('.');
		lcd_show_number(((unsigned char*)&ipaddr)[1]);	putChar_LCD('.');
		lcd_show_number(((unsigned char*)&ipaddr)[0]);
	}	
}
//Xuat ip ra LCD
void showip(void)
{
	clrline2lcd();
	move_LCD(2,1);	network_out(IpMyConfig.ip,1);
	//move_LCD(2,1);	network_out(IpMyConfig.netmask,2);
}
//----------------------------------------------------------------------------
void netInit(unsigned long ipaddress, unsigned long netmask, unsigned long gatewayip)
{
	ethInit();
	arpInit();
	ipSetConfig(ipaddress, netmask, gatewayip);
}
void	SystemInit()
{
	//LCD
	init_LCD();
	clr_LCD();
	
	//Set PORT
	DDRC = 0xff;	//out
	PORTC = 0xff;
	DDRB = 0x00;	//in	
	PORTB = 0xff;
	DDRD = 0x00;
	PORTD = 0xff;
	//Timer
	timer1Init();
	
	//UART
//	uartInit(UART_BAUDRATE);
	
	//RTC
	set_clock();
	TWI_DS1307_wadr(0x00); //set dia chi ve 0
	_delay_ms(1);			//cho DS1307 xu li 
	TWI_DS1307_rblock(tData,7); //doc ca khoi thoi gian (7 bytes)	
	Display(); // hien thi ket qua len LCD	
}
void EthernetInit()
{	
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
	if(IpMyConfig.ip == 0x00000000){
		dhcpInit();
	}
	TCPCreateSession(80,httpDataIn);
	httpInit();
}
//--------------------------------------------------------------------------------------
int	main()
{
	
	SystemInit();
	Backup_System();
	EthernetInit();
	while(1)
	{
		if(IpMyConfig.ip == 0x00000000)	dhcpInit();
		ethService();
		dhcpService();
	}
	return 0;
}