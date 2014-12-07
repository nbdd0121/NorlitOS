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
#include "mouse.h"

static u32 mouse_lock=0;
static Process* mouse_proc;

INITIALIZER FASTCALL void init_mouse(){
	registerIRQ(12, irq12_handler);
	enableIRQ(12);
	mouse_proc=createProcess((u32)mouse_bottom, 0, "Mouse",
			INPUT_DEVICE_PRIOPRITY);
	puts("[  0.000] MOUSE: Initialized.\r\n");
}

#define MSG_ADDR 0
#define INT_MSG_ADDR 0x1000

static ASMLINKAGE void mouse_bottom(){
	allocMemory(MSG_ADDR, 1);
	io_out8(CTL_PORT,0xA8);
	io_out8(CTL_PORT,0x60);
	io_out8(PS2_PORT,0x47);
	mouse_send(START_STREAMING);
	
	#if USE_WHEEL
		mouse_sendBytes(SET_SAMPLE_RATE,200);
		mouse_sendBytes(SET_SAMPLE_RATE,100);
		mouse_sendBytes(SET_SAMPLE_RATE,80);
		mouse_sendBytes(SET_SAMPLE_RATE,100);
		mouse_send(0xF2);
		u8 fb=mouse_read()==3;
	#endif	

	while(1){
		u8 b1=mouse_read();
		if(!BIT_TEST(b1,3))continue;
		if(BIT_TEST(b1,6)&&BIT_TEST(b1,7))continue;
		ms_msg.x=mouse_read();
		ms_msg.y=mouse_read();
		#if USE_WHEEL
			if(fb){
				ms_msg.scroll=mouse_read();
			}
		#endif
		
		ms_msg.button=b1&0b111;
		memcpy((void*)MSG_ADDR, &ms_msg, sizeof(MOUSE_MSG));
		sendMessage(PID_WINMAN, MSG_ADDR);
	}
}

static FASTCALL u32 mouse_send(u8 command){
	mouse_clear();
	io_out8(CTL_PORT, MOUSE);
	io_out8(PS2_PORT, command);
	return mouse_read()==ACK;
}

static FASTCALL u32 mouse_sendBytes(u8 command, u8 sub){
	mouse_clear();
	io_out8(CTL_PORT, MOUSE);
	io_out8(PS2_PORT, command);
	if(mouse_read()!=ACK)return 0;
	io_out8(PS2_PORT, sub);
	return mouse_read()==ACK;
}

static FASTCALL void mouse_clear(){
	sendMessage(mouse_proc->pid, MSG_ADDR);
	while(receiveMessage(INT_MSG_ADDR)!=mouse_proc->pid);
	DRIVER_ENTER_CRITICAL(mouse_lock);
	ms_buf.p_tail=ms_buf.p_head;
	ms_buf.count=0;
	DRIVER_LEAVE_CRITICAL(mouse_lock);
}

static FASTCALL u8 mouse_read(){
	while(receiveMessage(INT_MSG_ADDR)!=-1);
	u8 scan_code=*(ms_buf.p_tail);
	ms_buf.p_tail++;
	if(ms_buf.p_tail==ms_buf.buf+MS_BUFFER_SIZE){
		ms_buf.p_tail=ms_buf.buf;
	}
	/* Modify ms_buf.count should avoid
	 * being interrupted by the 
	 * mouse interrupt handler */
	DRIVER_ENTER_CRITICAL(mouse_lock);
	ms_buf.count--;
	DRIVER_LEAVE_CRITICAL(mouse_lock);
	return scan_code;
}

static ASMLINKAGE void irq12_handler(u32 id){
	ENTER_CRITICAL(mouse_lock);
	u8 scan_code=io_in8(PS2_PORT);
	if(ms_buf.count<MS_BUFFER_SIZE){
		*(ms_buf.p_head)=scan_code;
		ms_buf.p_head++;
		if(ms_buf.p_head==ms_buf.buf+MS_BUFFER_SIZE){
			ms_buf.p_head=ms_buf.buf;
		}
		ms_buf.count++;
	}
	LEAVE_CRITICAL(mouse_lock);
	sendInterrupt(mouse_proc);
}
