/*copyright by DongYingCai
Automation,	engineering of Computer and information institution 
Beijing Technology and Business University
*/

#include <reg51.h>
#include <intrins.h>

#define DA P0
#define lcd_d P1

#define uchar unsigned char
#define uint unsigned int

sbit DQ = P2^4;
sbit beep = P2^5;

sbit RS = P2^1;
sbit RW = P2^2;
sbit E = P2^3;

uchar presence;
uchar temp_data [2] = {0x00, 0x00};
uchar data disdata [4];
uint out_data;

bit flash = 0;


/**********************************
LCD 1602 
**********************************/
void ddelay (unsigned int n){
	unsigned int i, j;
	for (i = 0; i < n; i ++);
		for (j = 0; j< 100; j++);
}

void wr_comm (uchar comm){
	ddelay (1);
	RS = 0;
	RW = 0;
	E = 0;
	lcd_d = comm;
	ddelay (1);
	E = 1;
	ddelay (1);
	E = 0;
}

void wr_dat (uchar ddat){
	ddelay (1);
	RS = 1;
	RW = 0;
	E = 0;
	lcd_d = ddat;
	ddelay (1);
	E = 1;
	ddelay (1);
	E = 0;
}

void lcd_init (){
	ddelay (15);
	
	wr_comm (0x38);
	ddelay (5);

	wr_comm (0x08);
	ddelay (5);

	wr_comm (0x01);
	ddelay (5);

	wr_comm (0x06);
	ddelay (5);

	wr_comm (0x0C);
	ddelay (5);
}

void gener_char (uchar x, uchar y, uchar ddat){
	uchar address;
	if (y ==1){
		address = 0x80 + x;
	} else{
		address = 0xc0 + x;
	}
	wr_comm (address);
	wr_dat (ddat);
}

void disp_char (void){
	uchar letter [12] = {0x54, 0x65, 0x6d, 0x70, 0x65, 0x72,
						 0x61, 0x74, 0x75, 0x72, 0x65, 0x3a};
	uint i;
	lcd_init ();
	for (i = 0; i < 12; i ++){
		gener_char (i, 1, letter [i]);
	}
}


/*****************************
DS18B20 
*****************************/
void delay (uint num){
	while ( -- num );
}

uchar init_ds18b20 (void){
	DQ = 1;
	delay (8);

	DQ = 0;
	delay (90);

	DQ = 1;
	delay (8);

	presence = DQ;
	delay (100);
	DQ = 1;

	return (presence);
}

uchar readOneChar (void){
	uchar i;
	uchar value = 0;
	for ( i = 8; i > 0; i --){
		DQ = 1;
		_nop_(); _nop_();

		value >>= 1;
		
		DQ = 0;
		_nop_(); _nop_(); _nop_(); _nop_();

		DQ = 1;
		_nop_(); _nop_(); _nop_(); _nop_();

		if (DQ) value |= 0x80;
		delay (6);
	}
	DQ = 1;
	return (value);
}

void writeOneChar (uchar dat) {
	uchar i = 0;

	for (i = 8; i > 0; i --){
		DQ = 0;
		DQ = dat & 0x01;
		delay (5);

		DQ = 1;
		dat >>= 1;
	}
}

void beep_alert (){
	beep = 0;
	delay (250);
	beep = 1;
}

uint read_main (void){
	init_ds18b20 ();
	if (presence == 1){
		beep_alert ();
		flash = 1;
	} else {
		flash = 0;
		writeOneChar (0xCC);
		writeOneChar (0x44);

		init_ds18b20 ();
		writeOneChar (0xCC);
		writeOneChar (0xBE);

		temp_data [0] = readOneChar ();
		temp_data [1] = readOneChar ();

		temp_data [0] >>= 4;
		temp_data [1] <<= 4;
		
		out_data = temp_data [0] | temp_data [1];
	}	
	return (out_data);
}

/*******************************
Display tempertature
in the second line
*******************************/
void disp_temp (){
	disdata [0] = out_data% 100 / 10 + 0x30;
	disdata [1] = out_data% 10 + 0x30;

	if (disdata [0] == '0'){
		disdata [0] = ' ';
	}
 
    wr_comm(0xc1);
    wr_dat(disdata[0]); 
    wr_comm(0xc2);
    wr_dat(disdata[1]); 	  
}

/*****************************
Main 
*****************************/
void main (){
	lcd_init ();

	disp_char ();
	
	while (1) {
		read_main ();
		disp_temp ();
		DA = read_main ();
		delay (200);
	}
}