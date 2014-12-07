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
	 All constant in kernel should be defined here.
     The path of this source code is kernel/include/const.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/
 
#pragma once

#include "config.h"
 
#define GDT_LENGTH 6
#define SEL_FLAT_EMPTY 0
#define SEL_FLAT_C 8
#define SEL_FLAT_D 16
#define SEL_FLAT_C_USR 24
#define SEL_FLAT_D_USR 32
#define SEL_FLAT_TSS 40

#define SSA_RPL0 0
#define SSA_RPL3 3

#define IDT_LENGTH 256

#define DISP_OFFSET (PAGE_OFFSET+0xB8000)

#define SECTOR_SIZE		512

/* timer.c */
#define TIMER_FREQ 1193182L
#define TIMER0_PORT 0x40
#define TIMER_MODE_PORT 0x43
#define RATE_GENERATOR 0b00110100

#define CMOS_ADDRESS_REG	0x70
#define CMOS_DATA_REG	0x71

/* video.c */
#define videoMemory ((u8*)VIDEO_MEMORY_LINEAR)

/* paging.c */
#define PAGE_SIZE 0x1000

/* int.c */
#define INT_MASTER_CTL		0x20
#define INT_MASTER_CTLMASK	0x21
#define INT_SLAVE_CTL		0xA0
#define INT_SLAVE_CTLMASK	0xA1
#define INT_8259A_M_ICW1	0b00010001
#define INT_8259A_S_ICW1	0b00010001
#define INT_8259A_M_ICW2	0x20
#define INT_8259A_S_ICW2	0x28
#define INT_8259A_M_ICW3	0b00000100
#define INT_8259A_S_ICW3	2
#define INT_8259A_M_ICW4	0b00000001
#define INT_8259A_S_ICW4	0b00000001
#define PAGE_FAULT			14

