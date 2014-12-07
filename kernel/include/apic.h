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
     This file defines all APIC-related macros and constants.
     The path of this source code is kernel/include/apic.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include "typedef.h"
#include "global.h"

#define IA32_APIC_BASE	0x1B

#define APIC_IOREGSEL	IO_APIC_ADDR
#define APIC_IOWIN		(IO_APIC_ADDR+0x10)

#define IO_APIC_ID		0x0
#define IO_APIC_VER		0x1
#define IO_APIC_ARB		0x2
#define IO_APIC_REDTBL	0x10
#define IO_APIC_RED(x)	(IO_APIC_REDTBL+(x)*2)

#define APIC_ICR_LOW	0x300
#define APIC_ICR_HIGH	0x310

#define APIC_ID_REG		0x20
#define APIC_VER_REG	0x30
#define APIC_TPR		0x80
#define APIC_APR		0x90
#define APIC_PPR		0xA0
#define APIC_EOI_REG	0xB0
#define APIC_RRD		0xC0
#define APIC_LDR		0xD0
#define APIC_DFR		0xE0
#define APIC_SVR		0xF0
#define APIC_ESR		0x280
#define APIC_LVT_CMCI	0x2F0
#define APIC_LVT_TIMER	0x320
#define APIC_LVT_TSR	0x330
#define APIC_LVT_PMCR	0x340
#define APIC_LVT_LINT0	0x350
#define APIC_LVT_LINT1	0x360
#define APIC_LVT_ERR	0x370
#define APIC_TIMER_ICR	0x380
#define APIC_TIMER_CCR	0x390
#define APIC_TIMER_DCR	0x3E0
 
static INLINE void apic_write(u32 reg, u32 v){
	(*((volatile u32*)(APIC_REG_ADDR+(u32)(reg))) = v);
}

static INLINE u32 apic_read(u32 reg){
	return *((volatile u32*)(APIC_REG_ADDR+reg));
}

static INLINE u32 io_apic_read(u32 reg){
	*((volatile u32*)APIC_IOREGSEL)=reg;
	asm_mfence();
	return *((volatile u32*)APIC_IOWIN);
}

static INLINE u32 io_apic_write(u32 reg, u32 v){
	*((volatile u32*)APIC_IOREGSEL)=reg;
	asm_sfence();
	*((volatile u32*)APIC_IOWIN)=v;
}
