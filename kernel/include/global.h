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
     This file is the prototype header.
	 All C files in kernel should use the types defined by
		this file, in order to make use of symbols.
     The path of this source code is kernel/include/proto.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include "typedef.h"
#include "type.h"
#include "const.h"
#include "proto.h"

#ifdef EXTERN
u32* memoryBlocks[28]={EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,// 0-7 Bytes
							EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,// 8-17 KBs
							EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY};// 18-27 MBs
u32 irqOverride[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
#else
extern u32* memoryBlocks[28];
extern u32 irqOverride[16];
#endif

#ifndef EXTERN
#define EXTERN(x,y) extern x
#endif

EXTERN(u32* flatPTEs,NULL);
EXTERN(u32* kernelPDE,NULL);
EXTERN(volatile u64 wallClock,0);
EXTERN(u64 startupTime,0);
EXTERN(u32 freeMemoryCounter,0);
EXTERN(u32 scheduleLock,0);
EXTERN(LinkedList* processTable,NULL);
EXTERN(u8 videoBuffer[1024],{});
EXTERN(u8 *buffer,videoBuffer);
EXTERN(u32 winManagerPID,0);
#define cpuPage (*((KernelPage*)(KERNEL_STRUCTURE+PAGE_OFFSET)))
#define bootParam (*((BootParam*)BOOT_PARAM_POS))
#define vesaInfo (*((VESAControllerInfo*)VESA_PARAM_POS))




