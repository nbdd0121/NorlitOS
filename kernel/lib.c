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
     This file is the library for kernel.
     The path of this source code is kernel/lib.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "global.h"

/** These three function was borrowed from libgcc **/
FASTCALL u64 do_divmod64(u64 num, u64 den, u64 *rem_p){
	u64 quot=0, qbit=1;
	if (den==0) {
		return 1/((u32)den); /* Intentional divide by zero, without
								 triggering a compiler warning which
								 would abort the build */
	}
	/* Left-justify denominator and count shift */
	while((s64)den>=0){
		den <<= 1;
		qbit <<= 1;
	}
	while (qbit){
		if(den<=num){
			num-=den;
			quot+=qbit;
		}
		den>>=1;
		qbit>>=1;
	}
	if(rem_p)*rem_p=num;
	return quot;
}

FASTCALL u64 do_mod64(u64 num, u64 den){
	u64 v;
	do_divmod64(num, den, &v);
	return v;
}

FASTCALL u64 do_div64(u64 num, u64 den){
	return do_divmod64(num, den, NULL);
}

ASMLINKAGE void puts(const u8* str){
	const u8 *tmp = str;
	u8 c;
	while((c=*(tmp++))!=0){
		putc(c);
	}
}

ASMLINKAGE void* memcpy(void* dest, const void* src, u32 count){
    u8 *tmp = dest;
    const u8 *s = src;
    while (count--)
        *tmp++ = *s++ ;
    return dest;
}

ASMLINKAGE u8* strcpy(u8* dest, const u8* src){
    u8 *tmp = dest;
    const u8 *s = src;
    while (*s!=0)
        *tmp++ = *s++ ;
    *tmp=0;
    return dest;
}

ASMLINKAGE void* memset(void* dest, u8 ch, u32 count){
    u8 *tmp = dest;
    while (count--)
        *tmp++ = ch;
    return dest;
}

ASMLINKAGE u32 strlen(const u8* src){
    const u8 *tmp = src;
    u32 len=0;
    while (*tmp++)
        len++;
    return len;
}

FASTCALL u8 checksum(const void* dest, u32 n){
    const u8 *tmp=dest;
	u8 sum=0;
    while(n--)
        sum+=*tmp++;
    return sum;
}

FASTCALL u32 memcmp(const void* s1, const void *s2, u32 n){
    const u8 *tmp=s1;
    const u8 *s=s2;
    while(n--)
        if(*tmp++!=*s++)
			return *--tmp-*--s;
    return 0;
}

FASTCALL u8 BCD2BIN(u8 val){
	return ((val)&15)+((val)>>4)*10;
}

FASTCALL u8 BIN2BCD(u8 val){
	return (((val)/10)<<4) + (val)%10;
}

/** This function's idea was borrowed from
		linux but modified to suit Norlit OS **/
FASTCALL u64 mktime(u32 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec){
	if((s8)(mon-=2)<=0){
		mon+=12;
		year-=1;  
    }  
    return ((((u64)(year/4-year/100+year/400+367*mon/12+day)
		+year*365-337)*24+hour)*60+min)*60+sec;
}

FASTCALL void gmtime(u64 time, Time *ret){
	static u16 mday[]={0,31,59,90,120,151,181,212,243,273,304,334,365};
	u64 mod;
	time=do_divmod64(time,1000,&mod);
	ret->milli=mod;
	time=do_divmod64(time,60,&mod);
	ret->second=mod;
	time=do_divmod64(time,60,&mod);
	ret->minute=mod;
	time=do_divmod64(time,24,&mod);
	ret->hour=mod;
	do_divmod64(time+1,7,&mod);/* 1/1/1 is Monday */
	ret->weekday=mod;
	u32 year=do_divmod64(time,365,&time);
	s32 days=time-year/4+year/100-year/400+1;
	while(days<=0){
		days+=365+((!(year%4)&&(year%100))||!(year%400));
		year--;
	}
	ret->year=++year;
	if((!(year%4)&&(year%100))||!(year%400)){
		if(days==60){
			ret->month=2;
			ret->day=29;
			return;
		}else if(days>60)days--;
	}
	u8 bmon=days/30;
	u8 mon=days/31;
	if(bmon!=mon){
		if(days>mday[bmon])mon=bmon;
	}
	ret->month=mon+1;
	ret->day=days-mday[mon];
}

FASTCALL void delay(u32 millisec){
	u64 end=wallClock+millisec;
	while(wallClock<end);
}

FASTCALL u32 stringHash(u8 *str){
	u32 seed=31;
	u32 hash=0; 
	u8 c;
	while(c=*str++){  
		hash=(hash*seed)+c;
	} 
	return hash;
}

FASTCALL void readPort(u32 port, void* buffer, u32 count){
	asm volatile("movl %0, %%edx"::"m"(port));
	asm volatile("movl %0, %%edi"::"m"(buffer));
	asm volatile("movl %0, %%ecx"::"r"(count>>1));
	asm volatile("cld;rep insw":::"edi","memory");
}

FASTCALL void writePort(u32 port, void* buffer, u32 count){
	asm volatile("movl %0, %%edx"::"m"(port));
	asm volatile("movl %0, %%esi"::"m"(buffer));
	asm volatile("movl %0, %%ecx"::"r"(count>>1));
	asm volatile("cld;rep outsw":::"edi");
}
