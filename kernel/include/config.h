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

#define KERNEL_SIZE 40960
/** LINK: When you change this value,
		also change the value in loader.inc
		Makefile and judgement.sh */
#define KERNEL_OFFSET 0xC0100000

#define LOADER_OFFSET 0xC0008000
#define SMP_LOADER_OFFSET 0xC0009000
#define HDBOOT_OFFSET 0xC000A000
	
#define PAGE_OFFSET 0xC0000000
/** LINK: When you change this value,
		also change the value in loader.inc
		Makefile */
		
#define BOOT_PARAM_POS	(PAGE_OFFSET+0x500)
#define VESA_PARAM_POS	(PAGE_OFFSET+0x1000)

#define KERNEL_STRUCTURE 0x7000

/* timer.c */
#define HZ 100

/**
 * AP_TICKETS 
 * 
 * Through this macro, you can define how long an AP schedule its
 * processes and threads. If you set it to 0, that means that you
 * would like to schedule every HZ times a second. BSP will not
 * influenced by this value (Notice that set it to 0 may cause
 * inaccuracy on Bochs)
 * 
 * @default 0xA0000 */
#define AP_TICKETS 0xA0000

/* paging.c */
#define MAX_MAPPING 0x30000000
#define LOADER_MAPPED 0x1000000

#define PAGE_TYPE_PHY 0x200

/* proc.c */
#define THREAD_STRUCT_SIZE 0	// 2^0 pages, also kernel stacks' size
#define PROCESS_NAME_LENGTH 16
#define SYSCALL_SEND 0
#define SYSCALL_RECEIVE 1
#define SYSCALL_CREATE_THREAD 2
#define SYSCALL_DISPOSE_THREAD 3
#define SYSCALL_OPEN_PHYSICAL_MEMORY 4
#define SYSCALL_ALLOC_MEMORY 5
#define SYSCALL_NICE 6
#define SYSCALL_REJECT 7

/* keyboard.c */
#define KB_BUFFER_SIZE	0x100

/* mouse.c */
#define MS_BUFFER_SIZE 0x100
#define USE_WHEEL 1

/* video.c */
#define VIDEO_MEMORY_LINEAR 0x0

/* apic.c */
/**
 * APIC_REG_ADDR 
 * 
 * Through this macro, you can define the linear address of Local
 * APIC. The APIC driver will map Local APIC page to APIC_REG_ADDR.
 * Make sure that this page and IO_APIC_ADDR page are in same 4MB.
 * 
 * @default 0xFEE00000 */
#define APIC_REG_ADDR	0xFEE00000

/**
 * IO_APIC_ADDR 
 * 
 * Through this macro, you can define the linear address of I/O
 * APIC. The APIC driver will map I/O APIC windows to IO_APIC_ADDR.
 * Make sure that this page and APIC_REG_ADDR page are in same 4MB.
 * 
 * @default 0xFEC00000 */
#define IO_APIC_ADDR	0xFEC00000

/**
 * NO_SMP
 * 
 * Through this macro, you can define whether Norlit OS supports SMP or
 * not. The known problem of Norlit OS is that it cannot run in SMP
 * environment in Virtualbox.
 * 
 * @default 0xFEC00000 */
#define NO_SMP 1

#define MAX_MESSAGE_COUNT 0x100

#define PID_VIDEO	1
#define PID_HD		2
#define PID_WINMAN	3
#define PID_FS		4

#define EVENT_MOUSE 1
#define EVENT_KEYBOARD 2
#define COMMAND_HD 3

#define HD_INIT 0
#define HD_OPEN 1
#define HD_READ 2
#define HD_WRITE 3

#define HD_FAIL 0

#define MAGIC_NORLIT 0x6E
#define MAGIC_EXTENDED 0x5
#define MAGIC_GPT 0xEE

#define INPUT_DEVICE_PRIOPRITY 0xFF
#define DEVICE_DRIVER_PRIORITY 0xFF

#define DATA_BLOCK_PTRS 128
#define EXT_DATA_BLOCKS 256
#define FILE_NAME_SIZE 256
#define DIR_FILE_PTRS 256
#define META_DATA_LENGTH 512

#define STACK_TOP PAGE_OFFSET
#define STACK_BOTTOM (STACK_TOP-0x8000000)
#define FILE_MSG_PAGE 0xB0000000
