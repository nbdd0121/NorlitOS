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
     This file is the part dealing with keyboard.
     The path of this source code is kernel/driver/keyboard.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "asm.h"
#include "global.h"
#include "apic.h"
#include "keyboard.h"

#define CASE_BREAK(x) case x: kb_msg.shift=BIT_CLEAR(kb_msg.shift,BIT_##x); break
#define CASE_MAKE(x) case x: kb_msg.shift=BIT_SET(kb_msg.shift,BIT_##x); break

static u32 keyboard_lock=0;
static Process* keyboard_proc;

INITIALIZER FASTCALL void init_keyboard(){
	registerIRQ(1, irq1_handler);
	enableIRQ(1);
	keyboard_proc=createProcess((u32)keyboard_bottom, 0, "Keyboard",
			INPUT_DEVICE_PRIOPRITY);
	puts("[  0.000] KBD: Initialized.\r\n");
}

#define MSG_ADDR 0
#define INT_MSG_ADDR 0x1000

static ASMLINKAGE void keyboard_bottom(){
	allocMemory(0,1);
	keyboard_sendBytes(SET_LED, kb_msg.led);
	while(1){
		u8 scan_code=keyboard_read();
		u8 parsed;
		u8 make;
		if(scan_code==0xE0){
			scan_code=keyboard_read();
			make=scan_code&0x80;
			parsed=keymapE0[scan_code&0x7F];
		}else if(scan_code==0xE1){
			keyboard_read();
			scan_code=keyboard_read();
			parsed=PAUSE;
		}else{
			parsed=keymap[scan_code&0x7F];
		}
		if(!parsed)continue;
		make=scan_code&0x80;
		if(make){
			switch(parsed){
			CASE_BREAK(CTRL_L);
			CASE_BREAK(CTRL_R);
			CASE_BREAK(ALT_L);
			CASE_BREAK(ALT_R);
			CASE_BREAK(SHIFT_L);
			CASE_BREAK(SHIFT_R);
			CASE_BREAK(GUI_L);
			CASE_BREAK(GUI_R);	
			}
		}else{
			switch(parsed){
			case CAPS:
				kb_msg.led=BIT_SWITCH(kb_msg.led,2);
				keyboard_sendBytes(SET_LED, kb_msg.led);
				break;
			case NUMLOCK:
				kb_msg.led=BIT_SWITCH(kb_msg.led,1);
				keyboard_sendBytes(SET_LED, kb_msg.led);
				break;
			case SCRLOCK:
				kb_msg.led=BIT_SWITCH(kb_msg.led,0);
				keyboard_sendBytes(SET_LED, kb_msg.led);
				break;
			CASE_MAKE(CTRL_L);
			CASE_MAKE(CTRL_R);
			CASE_MAKE(ALT_L);
			CASE_MAKE(ALT_R);
			CASE_MAKE(SHIFT_L);
			CASE_MAKE(SHIFT_R);
			CASE_MAKE(GUI_L);
			CASE_MAKE(GUI_R);
			}
		}
		kb_msg.key=parsed;
		kb_msg.type=make;
		memcpy((void*)MSG_ADDR, &kb_msg, sizeof(KEYBOARD_MSG));
		sendMessage(PID_WINMAN, MSG_ADDR);
	}
}

static FASTCALL u32 keyboard_send(u8 command){
	keyboard_clear();
	io_out8(PS2_PORT, command);
	return keyboard_read()==ACK;
}

static FASTCALL u32 keyboard_sendBytes(u8 command, u8 sub){
	keyboard_clear();
	io_out8(PS2_PORT, command);
	if(keyboard_read()!=ACK)return 0;
	io_out8(PS2_PORT, sub);
	return keyboard_read()==ACK;
}

static FASTCALL void keyboard_clear(){
	sendMessage(keyboard_proc->pid, MSG_ADDR);
	while(receiveMessage(INT_MSG_ADDR)!=keyboard_proc->pid);
	DRIVER_ENTER_CRITICAL(keyboard_lock);
	kb_buf.p_tail=kb_buf.p_head;
	kb_buf.count=0;
	DRIVER_LEAVE_CRITICAL(keyboard_lock);
}

static FASTCALL u8 keyboard_read(){
	while(receiveMessage(INT_MSG_ADDR)!=-1);
	u8 scan_code=*(kb_buf.p_tail);
	kb_buf.p_tail++;
	if(kb_buf.p_tail==kb_buf.buf+KB_BUFFER_SIZE){
		kb_buf.p_tail=kb_buf.buf;
	}
	/* Modify kb_buf.count should avoid
	 * being interrupted by the 
	 * keyboard interrupt handler */
	DRIVER_ENTER_CRITICAL(keyboard_lock);
	kb_buf.count--;
	DRIVER_LEAVE_CRITICAL(keyboard_lock);
	return scan_code;
}

static ASMLINKAGE void irq1_handler(u32 id){
	ENTER_CRITICAL(keyboard_lock);
	u8 scan_code=io_in8(PS2_PORT);
	if(kb_buf.count<KB_BUFFER_SIZE){
		*(kb_buf.p_head)=scan_code;
		kb_buf.p_head++;
		if(kb_buf.p_head==kb_buf.buf+KB_BUFFER_SIZE){
			kb_buf.p_head=kb_buf.buf;
		}
		kb_buf.count++;
	}
	LEAVE_CRITICAL(keyboard_lock);
	sendInterrupt(keyboard_proc);
}
