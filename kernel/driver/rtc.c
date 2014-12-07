/********************************************************************

   Copyright 2012-2013 by Gary Guo - All Rights Reserved

   * All source code or binary file can only be used as personal
     study or research, but can not be used for business.
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
     
     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
     LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ********************************************************************
     This file is the driver of RTC.
     The path of this source code is kernel/driver/rtc.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#define USE_ACPI_TYPES

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "asm.h"
#include "global.h"

FASTCALL u8 CMOS_read(u32 reg){
	io_out8(CMOS_ADDRESS_REG, reg);
	return io_in8(CMOS_DATA_REG);
}

FASTCALL void CMOS_write(u32 reg, u8 value){
	io_out8(CMOS_ADDRESS_REG, reg);
	io_out8(CMOS_DATA_REG, value);
}

FASTCALL void write_RTC(u64 wall_clock){
	Time tmm;
	gmtime(wall_clock,&tmm);
	u8 registerB=CMOS_read(0x0B);
	u8 hour_correction=0;
	if(!BIT_TEST(registerB,1)){
		if(tmm.hour==0){
			tmm.hour=12;
			hour_correction=0x80;
		}else if(tmm.hour>12){
			tmm.hour-=12;
			hour_correction=0x80;
		}
	}
	if(!BIT_TEST(registerB,2)){
		tmm.second=BIN2BCD(tmm.second);
		tmm.minute=BIN2BCD(tmm.minute);
		tmm.hour=BIN2BCD(tmm.hour);
		tmm.day=BIN2BCD(tmm.day);
		tmm.month=BIN2BCD(tmm.month);
		tmm.year=BIN2BCD(tmm.year%100);
	}
	tmm.hour|=hour_correction;
	while(BIT_TEST(CMOS_read(0x0A),7));
	CMOS_write(0x00, 0);
	while(BIT_TEST(CMOS_read(0x0A),7));
	CMOS_write(0x02, tmm.minute);
	CMOS_write(0x04, tmm.hour);
	CMOS_write(0x06, tmm.weekday);
	CMOS_write(0x07, tmm.day);
	CMOS_write(0x08, tmm.month);
	CMOS_write(0x09, tmm.year);
}

FASTCALL u64 read_RTC(){
	u8 last_second, last_minute, last_hour, last_day, last_month, last_year;
	u8 second, minute, hour, day, month;
	u8 hour_correction=0;
	u8 registerB;
	u16 year;
	while(BIT_TEST(CMOS_read(0x0A),7));
	second=CMOS_read(0x00);
	minute=CMOS_read(0x02);
	hour=CMOS_read(0x04);
	day=CMOS_read(0x07);
	month=CMOS_read(0x08);
	year=CMOS_read(0x09);
	do {
		last_second = second;
		last_minute = minute;
		last_hour = hour;
		last_day = day;
		last_month = month;
		last_year = year;
		while(BIT_TEST(CMOS_read(0x0A),7));
		second=CMOS_read(0x00);
		minute=CMOS_read(0x02);
		hour=CMOS_read(0x04);
		day=CMOS_read(0x07);
		month=CMOS_read(0x08);
		year=CMOS_read(0x09);
	}while((last_second!=second)||(last_minute!=minute)||(last_hour!=hour)||
		(last_day!=day)||(last_month!=month)||(last_year!=year));
	
	registerB=CMOS_read(0x0B);
	
	if((hour&0x80)&&(!BIT_TEST(registerB,1))){
		hour=hour&~0x80;
		hour_correction=12;
	}
	
	if(!BIT_TEST(registerB,2)){
		second=BCD2BIN(second);
		minute=BCD2BIN(minute);
		hour=BCD2BIN(hour);
		day=BCD2BIN(day);
		month=BCD2BIN(month);
		year=BCD2BIN(year);
	}
	
	hour=(hour+hour_correction)%24;
	year+=2000;
	return mktime(year,month,day,hour,minute,second)*1000;
}
