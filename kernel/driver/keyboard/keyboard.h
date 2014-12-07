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
	u8 buf[KB_BUFFER_SIZE];
}KB_BUFFER;

static ASMLINKAGE void irq1_handler(u32);
static ASMLINKAGE void keyboard_bottom();
static FASTCALL u8 keyboard_read();
static FASTCALL u32 keyboard_send(u8);
static FASTCALL u32 keyboard_sendBytes(u8, u8);
static FASTCALL void keyboard_clear();

static KB_BUFFER kb_buf={p_head:kb_buf.buf,p_tail:kb_buf.buf,count:0};

typedef struct{
	u32 signature;
	u8 type;
	u8 key;
	u8 led;
	/* Shift
	 * Bit 7:R GUI
	 * Bit 6:L GUI
	 * Bit 5:R SHIFT
	 * Bit 4:L SHIFT
	 * Bit 3:R ALT
	 * Bit 2:L ALT
	 * Bit 1:R CTRL
	 * Bit 0:L CTRL
	 * */
	u8 shift;
}KEYBOARD_MSG;

static KEYBOARD_MSG kb_msg={signature:EVENT_KEYBOARD,type:0,led:0,shift:0,key:0};

#define BIT_GUI_R 7
#define BIT_GUI_L 6
#define BIT_SHIFT_R 5
#define BIT_SHIFT_L 4
#define BIT_ALT_R 3
#define BIT_ALT_L 2
#define BIT_CTRL_R 1
#define BIT_CTRL_L 0

#define PS2_PORT 0x60
#define ACK 0xFA
#define SET_LED 0xED

#define F1 0x81
#define F2 0x82
#define F3 0x83
#define F4 0x84
#define F5 0x85
#define F6 0x86
#define F7 0x87
#define F8 0x88
#define F9 0x89
#define F10 0x8A
#define F11 0x8B
#define F12 0x8C
#define CAPS 0x8D
#define NUMLOCK 0x8E
#define SCRLOCK 0x8F
#define SHIFT_L 0x90
#define SHIFT_R 0x91
#define CTRL_L 0x92
#define CTRL_R 0x93
#define ALT_L 0x94
#define ALT_R 0x95
#define GUI_L 0x96
#define GUI_R 0x97

#define PAD_ADD 0x98
#define PAD_SUB 0x99
#define PAD_DOT 0x9A
#define PAD_ENTER 0x9B
#define PAD_DIV 0x9C
#define PAD_MUL 0x9D

#define PAD_0 0xA0
#define PAD_1 0xA1
#define PAD_2 0xA2
#define PAD_3 0xA3
#define PAD_4 0xA4
#define PAD_5 0xA5
#define PAD_6 0xA6
#define PAD_7 0xA7
#define PAD_8 0xA8
#define PAD_9 0xA9

#define APPS 0x9E
#define INSERT 0x9F
#define HOME 0xAA
#define END 0xAB
#define PGUP 0xAC
#define PGDN 0xAD
#define PRINTSCREEN 0xAE
#define PAUSE 0xAF

#define UP 0xB0
#define DOWN 0xB1
#define LEFT 0xB2
#define RIGHT 0xB3

#define NEXT_TRACK 0xC0
#define PREV_TRACK 0xC1
#define STOP 0xC2
#define PLAY_PAUSE 0xC3
#define MUTE 0xC4
#define VOL_UP 0xC5
#define VOL_DOWN 0xC6

static u8 keymap[0x80]={
/* 0x0 - None */ '\0',
/* 0x1 - ESC */ '\x1b',
/* 0x2 - 1 */ '1',
/* 0x3 - 2 */ '2',
/* 0x4 - 3 */ '3',
/* 0x5 - 4 */ '4',
/* 0x6 - 5 */ '5',
/* 0x7 - 6 */ '6',
/* 0x8 - 7 */ '7',
/* 0x9 - 8 */ '8',
/* 0xA - 9 */ '9',
/* 0xB - 0 */ '0',
/* 0xC - - */ '-',
/* 0xD - = */ '=',
/* 0xE - BACKSPACE */ '\b',
/* 0xF - TAB */ '\t',
/* 0x10 - q */ 'q',
/* 0x11 - w */ 'w',
/* 0x12 - e */ 'e',
/* 0x13 - r */ 'r',
/* 0x14 - t */ 't',
/* 0x15 - y */ 'y',
/* 0x16 - u */ 'u',
/* 0x17 - i */ 'i',
/* 0x18 - o */ 'o',
/* 0x19 - p */ 'p',
/* 0x1A - [ */ '[',
/* 0x1B - ] */ ']',
/* 0x1C - ENTER */ '\r',
/* 0x1D - L CTRL */ CTRL_L,
/* 0x1E - a */ 'a',
/* 0x1F - s */ 's',
/* 0x20 - d */ 'd',
/* 0x21 - f */ 'f',
/* 0x22 - g */ 'g',
/* 0x23 - h */ 'h',
/* 0x24 - j */ 'j',
/* 0x25 - k */ 'k',
/* 0x26 - l */ 'l',
/* 0x27 - ; */ ';',
/* 0x28 - \' */ '\'',
/* 0x29 - ` */ '`',
/* 0x2A - L SHIFT */ SHIFT_L,
/* 0x2B - \\ */ '\\',
/* 0x2C - z */ 'z',
/* 0x2D - x */ 'x',
/* 0x2E - c */ 'c',
/* 0x2F - v */ 'v',
/* 0x30 - b */ 'b',
/* 0x31 - n */ 'n',
/* 0x32 - m */ 'm',
/* 0x33 - , */ ',',
/* 0x34 - . */ '.',
/* 0x35 - / */ '/',
/* 0x36 - R SHIFT */ SHIFT_R,
/* 0x37 - * */ '*',
/* 0x38 - L ALT */ ALT_L,
/* 0x39 - ' ' */ ' ',
/* 0x3A - Caps */ CAPS,
/* 0x3B - F1 */ F1,
/* 0x3C - F2 */ F2,
/* 0x3D - F3 */ F3,
/* 0x3E - F4 */ F4,
/* 0x3F - F5 */ F5,
/* 0x40 - F6 */ F6,
/* 0x41 - F7 */ F7,
/* 0x42 - F8 */ F8,
/* 0x43 - F9 */ F9,
/* 0x44 - F10 */ F10,
/* 0x45 - NumLock */ NUMLOCK,
/* 0x46 - ScrLock */ SCRLOCK,
/* 0x47 - Pad 7 */ PAD_7,
/* 0x48 - Pad 8 */ PAD_8,
/* 0x49 - Pad 9 */ PAD_9,
/* 0x4A - Pad - */ PAD_SUB,
/* 0x4B - Pad 4 */ PAD_4,
/* 0x4C - Pad 5 */ PAD_5,
/* 0x4D - Pad 6 */ PAD_6,
/* 0x4E - Pad + */ PAD_ADD,
/* 0x4F - Pad 1 */ PAD_1,
/* 0x50 - Pad 2 */ PAD_2,
/* 0x51 - Pad 3 */ PAD_3,
/* 0x52 - Pad 0 */ PAD_0,
/* 0x53 - Pad . */ PAD_DOT,
/* 0x54 - ? */ '\0',
/* 0x55 - ? */ '\0',
/* 0x56 - ? */ '\0',
/* 0x57 - F11 */ F11,
/* 0x58 - F12 */ F12,
/* 0x59 - ? */ '\0',
/* 0x5A - ? */ '\0',
/* 0x5B - ? */ '\0',
/* 0x5C - ? */ '\0',
/* 0x5D - ? */ '\0',
/* 0x5E - ? */ '\0',
/* 0x5F - ? */ '\0',
/* 0x60 - ? */ '\0',
/* 0x61 - ? */ '\0',
/* 0x62 - ? */ '\0',
/* 0x63 - ? */ '\0',
/* 0x64 - ? */ '\0',
/* 0x65 - ? */ '\0',
/* 0x66 - ? */ '\0',
/* 0x67 - ? */ '\0',
/* 0x68 - ? */ '\0',
/* 0x69 - ? */ '\0',
/* 0x6A - ? */ '\0',
/* 0x6B - ? */ '\0',
/* 0x6C - ? */ '\0',
/* 0x6D - ? */ '\0',
/* 0x6E - ? */ '\0',
/* 0x6F - ? */ '\0',
/* 0x70 - ? */ '\0',
/* 0x71 - ? */ '\0',
/* 0x72 - ? */ '\0',
/* 0x73 - ? */ '\0',
/* 0x74 - ? */ '\0',
/* 0x75 - ? */ '\0',
/* 0x76 - ? */ '\0',
/* 0x77 - ? */ '\0',
/* 0x78 - ? */ '\0',
/* 0x79 - ? */ '\0',
/* 0x7A - ? */ '\0',
/* 0x7B - ? */ '\0',
/* 0x7C - ? */ '\0',
/* 0x7D - ? */ '\0',
/* 0x7E - ? */ '\0',
/* 0x7F - ? */ '\0',
};

static u8 keymapE0[0x80]={
/* 0x0 - ? */ '\0',
/* 0x1 - ? */ '\0',
/* 0x2 - ? */ '\0',
/* 0x3 - ? */ '\0',
/* 0x4 - ? */ '\0',
/* 0x5 - ? */ '\0',
/* 0x6 - ? */ '\0',
/* 0x7 - ? */ '\0',
/* 0x8 - ? */ '\0',
/* 0x9 - ? */ '\0',
/* 0xA - ? */ '\0',
/* 0xB - ? */ '\0',
/* 0xC - ? */ '\0',
/* 0xD - ? */ '\0',
/* 0xE - ? */ '\0',
/* 0xF - ? */ '\0',
/* 0x10 - ? */ PREV_TRACK,
/* 0x11 - ? */ '\0',
/* 0x12 - ? */ '\0',
/* 0x13 - ? */ '\0',
/* 0x14 - ? */ '\0',
/* 0x15 - ? */ '\0',
/* 0x16 - ? */ '\0',
/* 0x17 - ? */ '\0',
/* 0x18 - ? */ '\0',
/* 0x19 - ? */ NEXT_TRACK,
/* 0x1A - ? */ '\0',
/* 0x1B - ? */ '\0',
/* 0x1C - Pad Enter */ PAD_ENTER,
/* 0x1D - R CTRL */ CTRL_R,
/* 0x1E - ? */ '\0',
/* 0x1F - ? */ '\0',
/* 0x20 - ? */ MUTE,
/* 0x21 - ? */ '\0',
/* 0x22 - ? */ PLAY_PAUSE,
/* 0x23 - ? */ '\0',
/* 0x24 - ? */ STOP,
/* 0x25 - ? */ '\0',
/* 0x26 - ? */ '\0',
/* 0x27 - ? */ '\0',
/* 0x28 - ? */ '\0',
/* 0x29 - ? */ '\0',
/* 0x2A - ? */ '\0',
/* 0x2B - ? */ '\0',
/* 0x2C - ? */ '\0',
/* 0x2D - ? */ '\0',
/* 0x2E - ? */ VOL_DOWN,
/* 0x2F - ? */ '\0',
/* 0x30 - ? */ VOL_UP,
/* 0x31 - ? */ '\0',
/* 0x32 - ? */ '\0',
/* 0x33 - ? */ '\0',
/* 0x34 - ? */ '\0',
/* 0x35 - Pad / */ PAD_DIV,
/* 0x36 - ? */ '\0',
/* 0x37 - ? */ PRINTSCREEN,
/* 0x38 - R ALT */ ALT_R,
/* 0x39 - ? */ '\0',
/* 0x3A - ? */ '\0',
/* 0x3B - ? */ '\0',
/* 0x3C - ? */ '\0',
/* 0x3D - ? */ '\0',
/* 0x3E - ? */ '\0',
/* 0x3F - ? */ '\0',
/* 0x40 - ? */ '\0',
/* 0x41 - ? */ '\0',
/* 0x42 - ? */ '\0',
/* 0x43 - ? */ '\0',
/* 0x44 - ? */ '\0',
/* 0x45 - ? */ '\0',
/* 0x46 - ? */ '\0',
/* 0x47 - Home */ HOME,
/* 0x48 - Up */ UP,
/* 0x49 - PgUp */ PGUP,
/* 0x4A - ? */ '\0',
/* 0x4B - Left */ LEFT,
/* 0x4C - ? */ '\0',
/* 0x4D - Right */ RIGHT,
/* 0x4E - ? */ '\0',
/* 0x4F - End */ END,
/* 0x50 - Down */ DOWN,
/* 0x51 - PgDn */ PGDN,
/* 0x52 - Insert */ INSERT,
/* 0x53 - Delete */ '\x7F',
/* 0x54 - ? */ '\0',
/* 0x55 - ? */ '\0',
/* 0x56 - ? */ '\0',
/* 0x57 - ? */ '\0',
/* 0x58 - ? */ '\0',
/* 0x59 - ? */ '\0',
/* 0x5A - ? */ '\0',
/* 0x5B - L GUI */ GUI_L,
/* 0x5C - R GUI */ GUI_R,
/* 0x5D - Apps */ APPS,
/* 0x5E - ? */ '\0',
/* 0x5F - ? */ '\0',
/* 0x60 - ? */ '\0',
/* 0x61 - ? */ '\0',
/* 0x62 - ? */ '\0',
/* 0x63 - ? */ '\0',
/* 0x64 - ? */ '\0',
/* 0x65 - ? */ '\0',
/* 0x66 - ? */ '\0',
/* 0x67 - ? */ '\0',
/* 0x68 - ? */ '\0',
/* 0x69 - ? */ '\0',
/* 0x6A - ? */ '\0',
/* 0x6B - ? */ '\0',
/* 0x6C - ? */ '\0',
/* 0x6D - ? */ '\0',
/* 0x6E - ? */ '\0',
/* 0x6F - ? */ '\0',
/* 0x70 - ? */ '\0',
/* 0x71 - ? */ '\0',
/* 0x72 - ? */ '\0',
/* 0x73 - ? */ '\0',
/* 0x74 - ? */ '\0',
/* 0x75 - ? */ '\0',
/* 0x76 - ? */ '\0',
/* 0x77 - ? */ '\0',
/* 0x78 - ? */ '\0',
/* 0x79 - ? */ '\0',
/* 0x7A - ? */ '\0',
/* 0x7B - ? */ '\0',
/* 0x7C - ? */ '\0',
/* 0x7D - ? */ '\0',
/* 0x7E - ? */ '\0',
/* 0x7F - ? */ '\0',
};
