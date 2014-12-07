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
     This file is the constant definition header.
	 This file is for keyboard.
     The path of this source code is kernel/driver/keyboard/keyboard.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/
 
#pragma once

#include "config.h"

#pragma pack(1)

typedef struct{
	u8* p_head;
	u8* p_tail;
	volatile u32 count;
	u8 buf[MS_BUFFER_SIZE];
}MS_BUFFER;

static ASMLINKAGE void irq12_handler(u32);
static ASMLINKAGE void mouse_bottom();
static FASTCALL u8 mouse_read();
static FASTCALL u32 mouse_send(u8);
static FASTCALL u32 mouse_sendBytes(u8, u8);
static FASTCALL void mouse_clear();

static MS_BUFFER ms_buf={p_head:ms_buf.buf,p_tail:ms_buf.buf,count:0};

typedef struct{
	u32 signature;
	s8 x;
	s8 y;
	/* Shift
	 * Bit 2:M Button
	 * Bit 1:R button
	 * Bit 0:L Button
	 * */
	u8 button;
	u8 scroll;
}MOUSE_MSG;

static MOUSE_MSG ms_msg={signature:EVENT_MOUSE,x:0,y:0,button:0,scroll:0};

#define PS2_PORT 0x60
#define MOUSE 0xD4
#define CTL_PORT 0x64
#define ACK 0xFA
#define START_STREAMING 0xF4
#define SET_SAMPLE_RATE 0xF3
