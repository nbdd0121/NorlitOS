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
     This file is the library for kernel to form a output.
     The path of this source code is kernel/format.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"

#define LEFT 1
#define PLUS 2
#define SPACE 4
#define SPECIAL 8
#define ZEROPAD 16
#define SIGN 32
#define SMALL 64

ASMLINKAGE u32 printf(const u8* fmt, ...){
	u32 i;
	u8 buf[256];
	
	u8* args=(u8*)(&fmt)+4;
	i=vsprintf(buf, fmt, args);
	buf[i]=0;
	puts(buf);
	
	return i;
}

ASMLINKAGE u32 sprintf(u8* buf, const u8* fmt, ...){
	u32 i;
	u8* args=(u8*)(&fmt)+4;
	i=vsprintf(buf, fmt, args);
	buf[i]=0;
	return i;
}

FASTCALL u8* itoa(s32 num,u8 *str,s32 radix, s32 width, s32 precision, s32 type){
	const u8* index=(type&SMALL)?"0123456789abcdefghijklmnopqrstuvwxyz":"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	u32 unum;
	s32 i=0,j,k;
	if((type&(SIGN|PLUS))&&num<0){
		unum=(u32)-num;
		str[i++]='-';
	}else if(type&PLUS){
		unum=(u32)num;
		str[i++]='+';
	}else
		unum=(u32)num;
	if(type&SPECIAL){
		if(radix==8)str[i++]='0';
		else if(radix==16){
			str[i++]='0';
			str[i++]=(type&SMALL)?'x':'X';
		}
	}
	do{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	if(str[0]=='-'||str[0]=='+')
		k=1;
	else k=0;
	k+=(type&SPECIAL)?(radix==8?1:(radix==16?2:0)):0;
	u8 temp;
	if(!(type&LEFT))
		while(i<width){
			str[i++]=(type&ZEROPAD)?'0':' ';
		}
	for(j=k;j<=(i+k-1)/2;j++)
	{
		temp=str[j];
		str[j] = str[i-1+k-j];
		str[i-j-1+k]=temp;
	}
	while(i<width){
		str[i++]=' ';
	}
	return str+i;
}

ASMLINKAGE u32 vsprintf(u8* buf, const u8* fmt, u8* args){
	u8* p;
	u8 tmp[256];
	u8* arg=args;

	for(p=buf;*fmt!=0;fmt++){
		if(*fmt!='%'){*p++=*fmt;continue;}
		
		s32 flags=0;
		while(1){
			fmt++;
			switch(*fmt){
				case '-':flags|= LEFT;continue;
				case '+':flags|= PLUS;continue;
				case ' ':flags|= SPACE;continue;
				case '#':flags|= SPECIAL;continue;
				case '0':flags|= ZEROPAD;continue;
			}
			break;
		}
			
		s32 field_width=-1;
		if(*fmt>='0'&&*fmt<='9'){
			field_width=*fmt-'0';
			fmt++;
		}else if (*fmt == '*') {
			field_width=*((s32*)arg);
			if(field_width<0) {
				field_width=-field_width;
				flags|=LEFT;
			}
		}
		
		s32 precision=-1;
		if(*fmt=='.'){
			fmt++;
			if(*fmt>='0'&&*fmt<='9'){
				precision=*fmt-'0';
				fmt++;
			}else if(*fmt=='*'){
				precision=*((s32*)arg);
			}
			if(precision<0)
				precision=0;
		}

		s32 qualifier=-1;
			if (*fmt=='h'||*fmt=='l'||*fmt=='L'){
			qualifier=*fmt;
			++fmt;
		}

		switch(*fmt){
			case 'x':flags|=SMALL;
			case 'X':
				p=itoa(*((u32*)arg), p, 16, field_width, precision, flags);
				arg+=4;
				break;
			case 'd':case 'i':flags|=SIGN;
			case 'u':
				p=itoa(*((s32*)arg), p, 10, field_width, precision, flags);
				arg+=4;
				break;
			case 'o':
				p=itoa(*((u32*)arg), p, 8, field_width, precision, flags); 
				arg+=4;
				break;
			case 'n':;
				s32* ip=*((s32**)arg);
				arg+=4;
				*ip=p-tmp;
				break;
			case 'p':
				if(field_width==-1){
					field_width=8;
					flags|=ZEROPAD;
				}
				p=itoa(*((s32*)arg), p, 16, field_width, precision, flags);
				arg+=4;
				break;
			case 'c':
				if (!(flags&LEFT))
					while (--field_width > 0)
						*p++ = ' ';
				*p++=*arg;
				arg+=4;
				while (--field_width > 0)
					*p++ = ' ';
				break;
			case 's':;
				u8* str=*((u8**)arg);
				arg+=4;
				s32 len=strlen(str);
				if(precision>=0&&len>precision)
					len=precision;
				if(!(flags&LEFT))
					while(len<field_width--)
						*p++=' ';
				memcpy(p,str,len);
				p+=len;
				while(len<field_width--)
					*p++=' ';
				break;
			case '%':
				*p++='%';
				break;
			default:
				*p++='%';
				fmt--;
				break;
		}
	}
	*p=0;
	return p-buf;
}

FASTCALL s32 atoi(u8* p){
	boolean neg=FALSE;
	s32 res=0;// 结果
	if(p[0]=='+'||p[0]=='-')
		neg=(*p++!='+');
	while(*p>='0'&&*p<='9')
		res=res*10+(*p++-'0');
	return neg?-res:res;
}
