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
     This file is the part dealing with linked list.
     The path of this source code is kernel/include/list.h
     Gary Guo, 2013, All right reserved.
 ********************************************************************/

#pragma once

#include "typedef.h"
#include "type.h"

/** 
* LIST_HEAD - initial a linked list
* @name:       the name of the list head.
* */ 
#define LIST_HEAD(name) struct list_head name = { &(name), &(name) }

/** 
* list_empty - judge the linked list is empty or not
* @head:        the list head to judge with.
* */
static INLINE int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/** 
* init_list - clean the list and make it to be a empty list
* @ptr:         the pointer of the list to empty.
* */ 
static INLINE void init_list(struct list_head *ptr)
{
	ptr->next=ptr;
	ptr->prev=ptr;
}

/** 
* list_insert - insert a node into a linked list.
* @newl:        the node to be inserted. 
* @prev:       the node before. 
* @next:       the node after.
* */ 
static INLINE void list_insert(struct list_head *newl, struct list_head *prev, struct list_head *next)
{
	prev->next=newl;
	newl->prev=prev;
	next->prev=newl;
	newl->next=next;
}

/** 
* list_add_(head|tail) - insert a new node to the (head|tail) of the list
* @newl:        the node to be inserted. 
* @head:       the list to contain the node. 
* */ 
static INLINE void list_add_head(struct list_head *newl, struct list_head *head){list_insert(newl,head,head->next);}
static INLINE void list_add_tail(struct list_head *newl, struct list_head *head){list_insert(newl,head->prev,head);}

/** 
* list_hide - remove a node from a list.
* @entry:      the node to be removed or to be hide. 
* */ 
static INLINE void list_hide(struct list_head *entry)
{
	entry->prev->next=entry->next;
	entry->next->prev=entry->prev;
}

/** 
* list_del - remove a node from a list and empty the node.
* @entry:      the node to be removed and empty. 
* */ 
static INLINE void list_del(struct list_head *entry){list_hide(entry);init_list(entry);}

/** 
* list_move_(head|tail) - move a node from the original list to the (head|tail) of the newl list
* @entry:      the node to be moved. 
* @head:       the newl list to contain the node. 
* */ 
static INLINE void list_move_head(struct list_head *entry, struct list_head *head){list_hide(entry);list_add_head(entry,head);}
static INLINE void list_move_tail(struct list_head *entry, struct list_head *head){list_hide(entry);list_add_tail(entry,head);}

static INLINE void list_concat(struct list_head *dest,struct list_head *src){
	dest->prev->next=src->next;
	src->next->prev=dest->prev;
	dest->prev=src->prev;
	src->prev->next=dest;
}

static INLINE void list_slice(struct list_head *left,struct list_head *right){
	left->prev->next=right->next;
	right->next->prev=left->prev;
	left->prev=right;
	right->next=left;
}


/** 
* container_of - cast a member of a structure out to the containing structure 
* @ptr:        the pointer to the member. 
* @type:       the type of the container struct this is embedded in. 
* @member:     the name of the member within the struct. 
* */ 
#define container_of(ptr, type, member) ({             \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);     \
         (type *)( (char *)__mptr - offsetof(type,member) );})

/** 
* list_entry - get the struct which containing list_head
* @ptr:        pointer to the list_head struct. 
* @type:       the parent struct type. 
* @member:     the name of the list_head field inside the struct. 
* */ 
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/** 
* list_for_each - list every node inside a list. The usage is the same as for inside the block.
* @entry:      the node. 
* @prefetch:   the node prefetched to keep it safe.
* @list:       the list head. 
* Warning:     this list is safe if and only if you do not remove node from the list or only remove the current node or the node previous. The safety is not guaranteed if you remove the next entry.
* */ 
#define list_for_each(entry, prefetch, list) \
for (entry = (list)->next, prefetch = entry->next; entry != (list); \
entry = prefetch, prefetch = entry->next)

