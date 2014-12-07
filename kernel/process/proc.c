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
     This file is the part dealing with process.
     The path of this source code is kernel/process/proc.c
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#include "typedef.h"
#include "proto.h"
#include "const.h"
#include "list.h"
#include "asm.h"
#include "type.h"
#include "global.h"

static FASTCALL Process* newProcess(u32 cr3, u32 ip, u32 flags, u8 *name, u32 priority);
static FASTCALL Thread* createThread(Process* , u32);
static FASTCALL void disposeThread(Thread*);
static FASTCALL void threadSleep(Thread*);
static FASTCALL void processSleep(Process*);

FASTCALL static void threadSleep(Thread* t){
	t->flag=1;
	if(--t->process->avl_thread==0){
		processSleep(t->process);
	}else{
		t->process->flag=0;
		if(cpuPage.current_thread=t){
			schedule(1);
		}
	}
}

FASTCALL static void threadAwake(Thread* t){
	t->flag=0;
	t->process->avl_thread++;
	if(t->process->flag==1){
		t->process->flag=0;
	}
}

FASTCALL static void processSleep(Process* t){
	t->flag=1;
	if(cpuPage.current_proc==t){
		schedule(1);
	}
}

FASTCALL static Thread* createThread(Process* proc, u32 ip){
	u32 sp=allocThreadStack(proc);
	if(sp==0)return NULL;
	Thread* thread=ALLOC_PAGES(THREAD_STRUCT_SIZE);
	{
		thread->regs.cs=SEL_FLAT_C_USR|SSA_RPL3;
		thread->regs.ds=thread->regs.es=
			thread->regs.fs=thread->regs.ss=
			thread->regs.gs=SEL_FLAT_D_USR|SSA_RPL3;
		thread->regs.ip=ip;
		thread->stacktop=thread->regs.sp=sp;
		thread->regs.flags=proc->privilege==0?0x3202:0x202;
	}	/* Init regs */
	{
		thread->process=proc;
		LinkedList *entry, *prefetch;
		Thread *pentry;
		u32 tid=0;
		ENTER_CRITICAL(scheduleLock);
		list_for_each(entry, prefetch, &proc->thread){
			pentry=list_entry(entry, Thread, list);
			if(pentry->threadID!=tid)break;
			tid++;
		}
		thread->threadID=tid;
		list_add_tail(&thread->list, entry);
		LEAVE_CRITICAL(scheduleLock);
	}	/* Init list and tid */
	{
		proc->avl_thread++;
		thread->flag=0;
	}
	return thread;
}

FASTCALL static Process* newProcess(u32 cr3, u32 ip, u32 privilege, u8 *name, u32 priority){
	Process* proc=malloc(sizeof(Process));
	
	proc->cr3=cr3;
	proc->privilege=privilege;
	proc->avl_thread=0;
	init_list(&proc->thread);
	
	Thread* t1=createThread(proc, ip);
	proc->current=t1;
	
	proc->flag=0;
	proc->msgCount=0;
	proc->msgHead=proc->messages;
	proc->msgTail=proc->messages;
	proc->receiving=0;
	memcpy(proc->name, name, PROCESS_NAME_LENGTH);
	proc->ticks=proc->priority=priority;
	
	ENTER_CRITICAL(scheduleLock);
	LinkedList *entry, *prefetch;
	Process *pentry;
	u32 pid=1;
	list_for_each(entry, prefetch, processTable){
		pentry=list_entry(entry, Process, list);
		if(pentry->pid!=pid)break;
		pid++;
	}
	proc->pid=pid;
	list_add_tail(&proc->list, entry);
	LEAVE_CRITICAL(scheduleLock);
	return proc;
}

FASTCALL static void disposeThread(Thread* thread){
	Process* proc=thread->process;
	//u32 sp=thread->regs.sp;
	ENTER_CRITICAL(scheduleLock);
	list_hide(&thread->list);
	if(thread->flag==2)proc->receiving--;
	LEAVE_CRITICAL(scheduleLock);
	recycleThreadStack(thread);
	if(thread->flag==0){
		proc->avl_thread--;
	}
	if(!list_empty(&proc->thread)){
		if(proc->current==thread){
			proc->current=list_entry(proc->thread.next,Thread,list);
			schedule(0);
		}
	}else{
		disposeProcess(proc);
	}
	FREE_PAGES(thread, THREAD_STRUCT_SIZE);
}

FASTCALL Process* createProcess(u32 ip, u32 privilege, u8 *name, u32 priority){
	u32 cr3=createProcessPDE(cpuPage.current_proc->cr3);
	return newProcess(cr3, (u32)ip, privilege, name, priority);
}

FASTCALL void disposeProcess(Process* proc){
	if(!list_empty(&proc->thread)){
		do{disposeThread(proc->current);}while(!list_empty(&proc->thread));
	}else{
		if(cpuPage.current_proc==proc)schedule(2);
		ENTER_CRITICAL(scheduleLock);
		list_hide(&proc->list);
		recycleProcessPDE(proc->cr3);
		u32 a=0;
		while(proc->msgCount!=0){
			recyclePhysicalPage((*proc->msgTail)&~0xFFF);
			/* TODO Notice that because pte no longer point to the page,
			 * If we try to FREE_PAGE the page, we will receive a page
			 * error. The solution to here is acomplish a new physical
			 * page allocating system */
			proc->msgTail++;
			if(proc->msgTail==proc->messages+KB_BUFFER_SIZE){
				proc->msgTail=proc->messages;
			}
			proc->msgCount--;
		}
	
		free(proc);
		LEAVE_CRITICAL(scheduleLock);
	}
}

ASMLINKAGE void FileTester(){
	delay(2000);
	u32 handle=fileOpen(PID_FS, "/test.txt");
	//printf("%d",handle);
	MetaData* md=(MetaData*)fileBuffer(PID_FS, handle);
	printf("File name %s",md->fileName);
	while(1){
		u32* data=(u32*)fileRead(PID_FS, handle, 0, 0);
		printf("%p", *data);
		(*data)++;
		fileWrite(PID_FS, handle, 0, 0);
	}
	EXIT();
}

ASMLINKAGE void ListFile(){
	allocMemory(0, 1);
	delay(2000);
	u32 handle=fileOpen(PID_FS, "/media");
	//printf("%d",handle);
	memcpy(NULL, fileRead(PID_FS, handle, 0, 0), 2048);
	u32* ptrs=NULL;
	u32 addr;
	while(addr=*ptrs++){
		
		printf("[%p]", addr);
		*ptrs++;
	}
	//printf("File name %s",md->fileName);
	//for(md->)
	EXIT();
}

ASMLINKAGE void Init(){
	io_cli();
	createProcess((u32)ListFile, 0, "FileTest",1);
	io_sti();
	while(1){NICE();}
}

ASMLINKAGE void InitAP(){
	bootParam.lock=0;
	while(1){
		NICE();
	}
}

static FASTCALL Process* getProcessById(u32 id){
	LinkedList *entry, *prefetch;
	Process *pentry;
	list_for_each(entry, prefetch, processTable){
		pentry=list_entry(entry, Process, list);
		if(pentry->pid==id)return pentry;
	}
	return NULL;
}

FASTCALL void schedule(u32 flag){
	u32 crpde=((u32*)pa2va(cpuPage.current_proc->cr3))[va2pde(PAGE_OFFSET)];
	ENTER_CRITICAL(scheduleLock);
	cpuPage.current_proc->flag=flag;
	
	LinkedList* ll=processTable;
	s32 greatest_ticks=0;
	Process* p_ready=NULL;
	
	while(p_ready==NULL){
		do{
			Process* p=list_entry(ll,Process,list);
			if(p->flag==0)
				if(p->ticks>greatest_ticks){
					greatest_ticks=p->ticks;
					p_ready=p;
				}
			ll=ll->next;
		}while(ll!=processTable);

		if(p_ready==NULL){
			ll=processTable;
			do{
				Process* p=list_entry(ll,Process,list);
				if(p->flag==0)
					p->ticks=p->priority;
				ll=ll->next;
			}while(ll!=processTable);
		}
	}
	
	cpuPage.current_proc=p_ready;
	
	cpuPage.current_proc->flag=2;
	
	LEAVE_CRITICAL(scheduleLock);
	
	((u32*)pa2va(cpuPage.current_proc->cr3))[va2pde(PAGE_OFFSET)]=crpde;
	cpuPage.current_thread=cpuPage.current_proc->current;
	cpuPage.tss.esp0=(u32)cpuPage.current_thread+sizeof(Thread);
}

INITIALIZER FASTCALL void init_proc_ap(){
	Process* init;
	u32* pde=ALLOC_PAGE();
	memcpy(pde, kernelPDE, PAGE_SIZE);
	cpuPage.handlerTable[PAGE_FAULT]=pageFaultHandler;
	
	init=newProcess(va2pa(pde), (u32)InitAP, 0, "InitAP", 1);
	init->flag=2;
	
	cpuPage.current_proc=init;
	cpuPage.current_thread=init->current;
	cpuPage.tss.esp0=(u32)cpuPage.current_thread+sizeof(Thread);
}

INITIALIZER FASTCALL void init_proc(){
	cpuPage.handlerTable[PAGE_FAULT]=pageFaultHandler;
	Process* init;
	{
		LIST_HEAD(ll);
		processTable=&ll;
		u32* pde=ALLOC_PAGE();
		memcpy(pde, kernelPDE, PAGE_SIZE);
		//printf("%p",(u32)ALLOC_PAGE());
		init=newProcess(va2pa(pde), (u32)Init, 0, "Init", 1);
		init->pid=0;
		init->list.prev=&init->list;
		init->list.next=&init->list;
		processTable=&init->list;
		init->flag=2;
	}
	cpuPage.current_proc=init;
	cpuPage.current_thread=init->current;
	cpuPage.tss.esp0=(u32)cpuPage.current_thread+sizeof(Thread);
}

FASTCALL void sendInterrupt(Process* p){
	ENTER_CRITICAL(scheduleLock);
	if(p->receiving){
		p->receiving--;
		LinkedList *entry, *prefetch;
		Thread *pentry;
		list_for_each(entry, prefetch, &p->thread){
			pentry=list_entry(entry, Thread, list);
			if(pentry->flag==2)break;
		}
		pentry->regs.ax=-1;
		open_physical_memory(p, allocPhysicalPage(), pentry->regs.cx, 1, 0);
		threadAwake(pentry);
	}else{
		if(p->msgCount<MAX_MESSAGE_COUNT){
			*p->msgHead=0xFFFFFFFF00000000|allocPhysicalPage();
			p->msgHead++;
			if(p->msgHead==p->messages+MAX_MESSAGE_COUNT){
				p->msgHead=p->messages;
			}
			p->msgCount++;
		}else{puts("Q");}
	}
	LEAVE_CRITICAL(scheduleLock);
}

ASMLINKAGE void system_call(u32 eax, u32 edx, u32 ecx, u32 ebx){
	switch(eax){
	case SYSCALL_SEND:
		/**
		 * @return status
		 * -2 Adress Not Exist
		 * -1 Process Not Exist
		 *  0 Target Process Buffer Full
		 *  1 Send Successfully
		 */
		{
			Process* current=cpuPage.current_proc;
			if(ebx==0)ebx=current->pid;
			if(ecx<PAGE_OFFSET&&(ecx&0xFFF)==0){
				u32* pde=(u32*)pa2va(current->cr3&~0xFFF);
				if(BIT_TEST(pde[va2pde(ecx)],0)){
					u32* pte=(u32*)pa2va(pde[va2pde(ecx)]&~0xFFF);
					u32 page=pte[va2pte(ecx)];
					if(BIT_TEST(page,0)&&!(page&PAGE_TYPE_PHY)){
						ENTER_CRITICAL(scheduleLock);
						Process* p=getProcessById(edx);
						if(!p){
							cpuPage.current_thread->regs.ax=-1;
							LEAVE_AND_RETURN(scheduleLock);
						}
						if(p->receiving){
							p->receiving--;
							LinkedList *entry, *prefetch;
							Thread *pentry;
							list_for_each(entry, prefetch, &p->thread){
								pentry=list_entry(entry, Thread, list);
								if(pentry->flag==2)break;
							}
							pentry->regs.ax=ebx;
							open_physical_memory(p, page&~0xFFF, pentry->regs.cx, 1, 0);
							pte[va2pte(ecx)]=allocPhysicalPage()|0b11111;
							threadAwake(pentry);
						}else{
							if(p->msgCount<MAX_MESSAGE_COUNT){
								*p->msgHead=(u64)ebx<<32|page;
								//printf("{%p->%p->%p=%p=%c=%c}",pde,pte,page,pa2va(page&~0xFFF),*(u8*)pa2va(page&~0xFFF),*(u8*)ecx);
								pte[va2pte(ecx)]=allocPhysicalPage()|0b11111;
								p->msgHead++;
								if(p->msgHead==p->messages+MAX_MESSAGE_COUNT){
									p->msgHead=p->messages;
								}
								p->msgCount++;
								//putc('+');
							}else{
								cpuPage.current_thread->regs.ax=0;
								LEAVE_AND_RETURN(scheduleLock);
							}
						}
						cpuPage.current_thread->regs.ax=1;
						LEAVE_CRITICAL(scheduleLock);
						break;
					}
				}
			}
			cpuPage.current_thread->regs.ax=-2;
			break;
		}
	case SYSCALL_RECEIVE:
		/**
		 * @return source process identifier
		 * -2 Adress Not Exist
		 * -1 Hardware Interrupt
		 * >0 PID of Source Process
		 */
		{
			Process* p=cpuPage.current_proc;
			Thread* t=cpuPage.current_thread;
			if(ecx<PAGE_OFFSET&&(ecx&0xFFF)==0){
				u32* pde=(u32*)pa2va(cpuPage.current_proc->cr3&~0xFFF);
				if(BIT_TEST(pde[va2pde(ecx)],0)){
					u32* pte=(u32*)pa2va(pde[va2pde(ecx)]&~0xFFF);
					u32 page=pte[va2pte(ecx)];
					if(BIT_TEST(page,0)){
						if(page&PAGE_TYPE_PHY){
							cpuPage.current_thread->regs.ax=-2;
							break;
						}
						recyclePhysicalPage(page&~0xFFF);
						pte[va2pte(ecx)]=0;
					}
				}
				ENTER_CRITICAL(scheduleLock);
				if(p->msgCount==0){
					spin_unlock(&scheduleLock);
					threadSleep(t);
					t->flag=2;
					p->receiving++;
					//putc('~');
					break;
				}
				u64 msg=*p->msgTail;
				p->msgTail++;
				if(p->msgTail==p->messages+MAX_MESSAGE_COUNT){
					p->msgTail=p->messages;
				}
				p->msgCount--;
				//recyclePhysicalPage(msg&~0xFFF);
				//printf("%p[%p]",(u32)msg,(u8*)pa2va((u32)msg&~0xFFF));
				open_physical_memory(cpuPage.current_proc, (u32)msg&~0xFFF, ecx, 1, 0);
				cpuPage.current_thread->regs.ax=msg>>32;
				LEAVE_CRITICAL(scheduleLock);
				break;
			}
			cpuPage.current_thread->regs.ax=-2;
			break;
		}
	case SYSCALL_CREATE_THREAD:
		{
			/* No need to enter critical area because other CPU will
			 * not affect the current process and thread */
			cpuPage.current_thread->regs.ax=
				(u32)createThread(cpuPage.current_proc, edx);
			break;
		}
	case SYSCALL_DISPOSE_THREAD:
		{
			/* Switch stack, because the kernel stack of current thread
			 * will no longer existed */
			asm volatile("mov %0, %%esp"::"a"(cpuPage.kernelStack));
			/* No need to enter critical area because other CPU will
			 * not affect the current process and thread */
			Thread* thr=edx!=0?(Thread*)edx:cpuPage.current_thread;
			//if(thr->process==cpuPage.current_proc)
			disposeThread(thr);
			/* Manually call `restart` because no ret addr was pushed */
			restart();
		}
	case SYSCALL_OPEN_PHYSICAL_MEMORY:
		{
			cpuPage.current_thread->regs.ax=
				open_physical_memory(cpuPage.current_proc, edx, ecx, ebx, PAGE_TYPE_PHY);
			break;
		}
	case SYSCALL_ALLOC_MEMORY:
		{
			u32 phy=allocPhysicalPage();
			if((cpuPage.current_thread->regs.ax=
				open_physical_memory(cpuPage.current_proc, phy, ecx, ebx, 0))==-1){
				//FREE_PAGE(pa2va(phy));
				recyclePhysicalPage(phy);
			}
			break;
		}
	case SYSCALL_NICE:
		{
			Process* proc=cpuPage.current_proc;
			schedule(2);
			proc->flag=0;
			break;
		}
	default:
		{
			inner_exception_handler(22,-1);
		}
	}
}





