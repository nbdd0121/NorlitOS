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
     This file is the library for userspace processes.
     The path of this source code is kernel/proclib.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "global.h"

FASTCALL s32 send(u32 proc, u32 linear){
	s32 ret;
	while((ret=sendMessage(proc,linear))==0){
		NICE();
	}
	return ret;
}

FASTCALL s32 receive(u32 proc, u32 linear){
	s32 ret;
	//proc=0;
	while((ret=receiveMessage(linear))!=proc&&proc!=0){
		asm volatile("int $0x30"::"b"(ret),"c"(linear),"d"(cpuPage.current_proc->pid),"a"(SYSCALL_SEND):"memory");
	}
	return ret;
}

FASTCALL s32 sendReceive(u32 proc, u32 linear){
	s32 ret;
	if((ret=send(proc, linear))==1)
		return receive(proc, linear);
	else
		return ret;
}

s32 fileOpen(u32 proc, u8* name){
	FileMessage* msg=(FileMessage*)FILE_MSG_PAGE;
	if(allocMemory(FILE_MSG_PAGE, 1)!=-1){
		msg->signature=COMMAND_HD;
	}
	strcpy(msg->data, name);
	msg->command=HD_OPEN;
	msg->handle=stringHash(name);
	sendReceive(proc, FILE_MSG_PAGE);
	if(msg->command==HD_FAIL)return -1;
	return msg->handle;
}

u8* fileRead(u32 proc, u32 handle, u32 offset, u32 count){
	FileMessage* msg=(FileMessage*)FILE_MSG_PAGE;
	msg->command=HD_READ;
	msg->handle=handle;
	msg->offset=offset;
	msg->count=count;
	sendReceive(proc, FILE_MSG_PAGE);
	return msg->data;
}

u8* fileBuffer(u32 proc, u32 handle){
	return ((FileMessage*)FILE_MSG_PAGE)->data;
}

s32 fileWrite(u32 proc, u32 handle, u32 offset, u32 count){
	FileMessage* msg=(FileMessage*)FILE_MSG_PAGE;
	msg->command=HD_WRITE;
	msg->handle=handle;
	msg->offset=offset;
	msg->count=count;
	sendReceive(proc, FILE_MSG_PAGE);
}

