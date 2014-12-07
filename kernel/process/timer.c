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
     This file is the part dealing with timer.
     The path of this source code is kernel/process/timer.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "asm.h"
#include "global.h"
#include "apic.h"
#include "list.h"

#if !USING_APIC_TIMER && TIMER_FREQ/HZ > 65535
#error HZ must be greater than 18.
#endif

static ASMLINKAGE void irq0_handler(u32);
static ASMLINKAGE void irq0_handler_bsp(u32);
static INITIALIZER ASMLINKAGE void apic_timer_init(u32);

#if AP_TICKETS==0
static u32 time_count;
#endif

INITIALIZER FASTCALL void printStartupTime(){
	u32 tm=wallClock-startupTime;
	printf("[%3d.%03d]",tm/1000,tm%1000);
}

INITIALIZER FASTCALL void init_timer_ap(){
	apic_write(APIC_TIMER_DCR, 0x3);
	apic_write(APIC_LVT_TIMER, 0x20020);
	apic_write(APIC_TIMER_ICR, 
	#if AP_TICKETS==0
		time_count
	#else
		AP_TICKETS
	#endif
	);
	registerIRQ(0, irq0_handler);
}

INITIALIZER FASTCALL void init_timer(){
	startupTime=wallClock=read_RTC();
	io_out8(TIMER_MODE_PORT, RATE_GENERATOR);
	io_out8(TIMER0_PORT, (u8)(TIMER_FREQ/HZ));
	io_out8(TIMER0_PORT, (u8)((TIMER_FREQ/HZ)>>8));
	registerIRQ(0, apic_timer_init);
	apic_write(APIC_TIMER_DCR, 0x3);
	apic_write(APIC_LVT_TIMER, 0x20);
	apic_write(APIC_TIMER_ICR, 0xFFFFFFFF);
	enableIRQ(0);
	restart();
}

static INITIALIZER ASMLINKAGE void apic_timer_init(u32 id){
	#if AP_TICKETS==0
	time_count=0xFFFFFFFF-apic_read(APIC_TIMER_CCR);
	#endif
	registerIRQ(0, irq0_handler_bsp);
	bootParam.lock=0;
}

static ASMLINKAGE void irq0_handler_bsp(u32 id){
	wallClock+=1000/HZ;
	irq0_handler(id);
}

static ASMLINKAGE void irq0_handler(u32 id){
	/*LinkedList* ll=&cpuPage.current_thread->list;
	do{
		do{
			ll=ll->next;
		}while(ll==&cpuPage.current_proc->thread);
		cpuPage.current_thread=list_entry(ll,Thread,list);
	}while(cpuPage.current_thread->flag!=0);
	cpuPage.current_proc->current=cpuPage.current_thread;*/
	ENTER_CRITICAL(scheduleLock);//TODO
	/*if((--cpuPage.current_proc->ticks)>0){
		cpuPage.tss.esp0=(u32)cpuPage.current_thread+sizeof(Thread);
		LEAVE_AND_RETURN(scheduleLock);//TODO
		return;
	}*/
	--cpuPage.current_proc->ticks;
	LEAVE_CRITICAL(scheduleLock);//TODO
	schedule(0);
}
