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
     This file is the part dealing with video output.
     The path of this source code is kernel/video/video.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "asm.h"
#include "global.h"
#include "font.h"

static u16 xRes, yRes, bpl, bpp;

static FASTCALL void drawChar(u8 chr, u32 color, u16 x, u16 yn);
static FASTCALL void drawString(u8* string, u32 color, u16 x, u16 y);
static FASTCALL void drawPoint(u16, u16, u32);
static FASTCALL void fillRect(u32, u16, u16, u16, u16);
static FASTCALL void fillGradient(u32 color1, u32 color2, u16 x0, u16 y0, u16 x1, u16 y1, boolean hori);

volatile u8* volatile pointer=videoBuffer;

INITIALIZER FASTCALL void init_video(){
	buffer=ALLOC_PAGES(3);
	strcpy(buffer, videoBuffer);
	pointer+=buffer-videoBuffer;
	memory_free((u32)videoBuffer,sizeof(videoBuffer));
	*pointer=0;
	
	VESAModeInfo currentMode=vesaInfo.modeList[0];
	xRes=currentMode.width;
	yRes=currentMode.height;
	bpl=currentMode.bytesPerLine;
	bpp=currentMode.bitsPerPixel/8;
	printf("[  0.000] VIDEO: %.4s %d.%d, Memory: %dMB\r\n",
	vesaInfo.signature,
	vesaInfo.majorVersion,vesaInfo.minorVersion,
	vesaInfo.totalMemory*64/1024);
	printf("[  0.000] VIDEO: OEM: %s\r\n",rmfarptr2ptr(vesaInfo.oemString)+PAGE_OFFSET);
	VESAModeInfo *vmi=&vesaInfo.modeList[0];
	printf("[  0.000] VIDEO: Current Mode: Bit %d, %4dx%4d\r\n",
				vmi->bitsPerPixel,
				vmi->width,vmi->height);
	/*	u32 index;
		for(index=0;index<vesaInfo.modeCount;index++){
			VESAModeInfo *vmi=&vesaInfo.modeList[index];
			printStartupTime();
			if(index==0)puts(" {VD} Current Mode:");
			else puts(" {VD} Alternative Mode:");
			printf(" Bit %d, %4dx%4d\r\n",
				vmi->bitsPerPixel,
				vmi->width,vmi->height);
		}*/
	
	Process* proc=createProcess((u32)video_driver, 0, "Video", 2);
}
ASMLINKAGE void video_driver(){
	VESAModeInfo currentMode=vesaInfo.modeList[0];
	openPhysicalMemory(currentMode.physbase, VIDEO_MEMORY_LINEAR, vesaInfo.totalMemory*64/4);
	/*{
		u16 centerPanelx=(xRes-160)/2;
		u16 centerPanely=(yRes-32)/2;
		fillGradient(0x5A574E,0x3F3E39, 0, 0, xRes, yRes, FALSE);
		drawString("Norlit OS Beta V0.000001", 0xFFFFFF, centerPanelx-16, centerPanely);
		fillRect(0xFFFFFF, centerPanelx, centerPanely+20, centerPanelx+160, centerPanely+32);
		drawString("Copyright (C): \x80\x81\x82\x83", 0xFFFFFF, centerPanelx+4, yRes-20);
		u8 ld;
		for(ld=0;ld<156;ld+=4){
			fillRect(0, centerPanelx+2, centerPanely+22, centerPanelx+2+4, centerPanely+30);
			centerPanelx+=4;
			delay(100);
		}
	}*/
	fillGradient(0x5A574E,0x3F3E39, 0, 0, xRes, 24, FALSE);
	fillRect(0x300A24, 0, 24, xRes, yRes);
	drawString("Norlit OS", 0xFFFFFF, 6, 4);
	s16 x=4;
	s16 y=28;
	while(1){
		while(pointer==buffer);
		{
			u8 a[12];
			u32 k[]={freeMemoryCounter/1024/1024,freeMemoryCounter%(1024*1024)/1024,freeMemoryCounter%1024};
			u32* b=k;
			vsprintf(a, "%dMB%4dKB%4dBytes", (u8*)b);
			fillGradient(0x5A574E,0x3F3E39, 8*13, 4, 8*34, 16+4, FALSE);
			drawString(a, 0xFFFFFF,8*13,4);
		}
		volatile u8 * volatile tmp=buffer;
		u8 c;
		while((c=*(tmp++))!=0){
			switch(c){
			case '\r':x=4;break;
			case '\n':y+=16;break;
			case '\t':x=x-(x-4)%64+64;if((s32)(xRes-x)<64){x=4;y+=16;}break;
			case '\b':x-=8;break;
			case '\f':x=4;y=28;fillRect(0x300A24, 0, 24, xRes, yRes);break;
			case '\1':y-=16;break;
			default:
				fillRect(0x300A24, x, y, x+8, y+16);
				drawChar(c, 0xFFFFFF, x, y);
				x+=8;
			}
			if(x>=(s16)(xRes-4)){x-=xRes-8;y+=16;}
			else if(x<4){x+=xRes-8;y-=16;}
			if(y<28)y=28;
			else if(y>(s32)(yRes-16))y-=16;
		}
		pointer=buffer;
	}
}

static FASTCALL void drawString(u8* string, u32 color, u16 x, u16 y){
	u8* tmp=string;
	u8 c;
	while((c=*(tmp++))!=0){
		switch(c){
		case '\r':x=0;break;
		case '\n':y+=16;break;
		case '\t':x+=8*8;break;
		case '\b':if(x==0){x=xRes-8;y-=16;}else x-=8;break;
		case '\f':x=y=0;break;
		case '\1':y-=16;if(y<24)y=24;break;
		default:
			drawChar(c, color, x, y);
			x+=8;
			if(x>=xRes){x-=xRes;y+=16;}
		}
	}
}

static FASTCALL void fillRect(u32 color, u16 x0, u16 y0, u16 x1, u16 y1){
	u16 x, y;
	for(y=y0;y<y1;y++){
		for(x=x0;x<x1;x++){
			drawPoint(x, y, color);
		}
	}
}

static FASTCALL void fillGradient(u32 color1, u32 color2, u16 x0, u16 y0, u16 x1, u16 y1, boolean hori){
	u16 x, y;
	u8 r=color1>>16;
	u8 g=color1>>8;
	u8 b=color1;
	s16 r1=(u8)(color2>>16)-r;
	s16 g1=(u8)(color2>>8)-g;
	s16 b1=(u8)color2-b;
	if(hori){
		x1=x1-x0;
		for(x=0;x<x1;x++){
			u32 color=((x*r1/x1+r)<<16)+((x*g1/x1+g)<<8)+(x*b1/x1+b);
			for(y=y0;y<y1;y++){
				drawPoint(x+x0, y, color);
			}
		}		
	}else{
		y1=y1-y0;
		for(y=0;y<y1;y++){
			u32 color=((y*r1/y1+r)<<16)+((y*g1/y1+g)<<8)+(y*b1/y1+b);
			for(x=x0;x<x1;x++){
				drawPoint(x, y+y0, color);
			}
		}
	}
}

static FASTCALL void drawChar(u8 chr, u32 color, u16 x, u16 yn){
	u8* font=&font_ascii[chr*16];
	u32 index;
	for(index=0;index<16;index++,yn++){
		u32 xn=x;
		if(BIT_TEST(*font,7))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,6))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,5))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,4))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,3))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,2))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,1))drawPoint(xn,yn,color);xn++;
		if(BIT_TEST(*font,0))drawPoint(xn,yn,color);
		font++;
	}
}

static FASTCALL void drawPoint(u16 x, u16 y, u32 rgb){
	u8 *pos=&videoMemory[y*bpl+x*bpp];
	u8 *clr=(u8*)&rgb;
	pos[0]=clr[0];
	pos[1]=clr[1];
	pos[2]=clr[2];
}

ASMLINKAGE void putc(u8 chr){
	*pointer++=chr;
	*pointer=0;
}
