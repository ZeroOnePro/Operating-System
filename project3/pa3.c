/**********************************************************************
 * Copyright (c) 2019
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "types.h"
#include "list_head.h"
#include "vm.h"

/**
 * Ready queue of the system
 */
extern struct list_head processes;

/**
 * The current process
 */
extern struct process *current;

/**
 * alloc_page()
 *
 * DESCRIPTION
 *   Allocate a page from the system. This function is implemented in vm.c
 *   and use to get a page frame from the system.
 *
 * RETURN
 *   PFN of the newly allocated page frame.
 */
extern unsigned int alloc_page(void);
struct pte_directory pd[NR_PTES_PER_PAGE];

struct process forked[4] ={{0,NULL},{0,NULL},{0,NULL},{0,NULL}};


/**
 * TODO translate()
 *
 * DESCRIPTION
 *   Translate @vpn of the @current to @pfn. To this end, walk through the
 *   page table of @current and find the corresponding PTE of @vpn.
 *   If such an entry exists and OK to access the pfn in the PTE, fill @pfn
 *   with the pfn of the PTE and return true.
 *   Otherwise, return false.
 *   Note that you should not modify any part of the page table in this function.
 *
 * RETURN
 *   @true on successful translation
 *   @false on unable to translate. This includes the case when @rw is for write
 *   and the @writable of the pte is false.
 */
bool translate(enum memory_access_type rw, unsigned int vpn, unsigned int *pfn)
{
	/*** DO NOT MODIFY THE PAGE TABLE IN THIS FUNCTION ***/
	
	int outer_index = vpn / 16;
	int inner_index = vpn % 16;

	if(!current->pagetable.outer_ptes[outer_index]){ // initial
		 return false;
	}else if(!current->pagetable.outer_ptes[outer_index]->ptes[inner_index].valid){ // inner page table valid = false
		return false;
	}else{
		*pfn = current->pagetable.outer_ptes[outer_index]->ptes[inner_index].pfn;
		return true;
	}
}


/**
 * TODO handle_page_fault()
 *
 * DESCRIPTION
 *   Handle the page fault for accessing @vpn for @rw. This function is called
 *   by the framework when the translate() for @vpn fails. This implies;
 *   1. Corresponding pte_directory is not exist
 *   2. pte is not valid
 *   3. pte is not writable but @rw is for write
 *   You can assume that all pages are writable; this means, when a page fault
 *   happens with valid PTE without writable permission, it was set for the
 *   copy-on-write.
 *
 * RETURN
 *   @true on successful fault handling
 *   @false otherwise
 */
bool handle_page_fault(enum memory_access_type rw, unsigned int vpn)
{
	// 이미 할당된것이 없다는 것을 알고 왔으므로
	// 새로 할당할 것
	// pte->pfn에 저장해주면 끝
	// vpn 0-255, outer_ptes 16, ptes 16
	// make new pte
	int outer_index = vpn / 16;
	int inner_index = vpn % 16;

	
	// demand paging
	// make new inner page table
	if(!pd[outer_index].ptes[inner_index].valid){
		 pd[outer_index].ptes[inner_index].pfn = alloc_page(); // if not vaild, alloc_page
		 pd[outer_index].ptes[inner_index].writable = true; // writable bit = on
		pd[outer_index].ptes[inner_index].valid = true; // if page allocate, this pte is valid..
	}
	
	
	// make new outer page table and MAP
	current->pagetable.outer_ptes[outer_index] = &pd[outer_index];
	
	return true;
}


/**
 * TODO switch_process()
 *
 * DESCRIPTION
 *   If there is a process with @pid in @processes, switch to the process.
 *   The @current process at the moment should be put to the **TAIL** of the
 *   @processes list, and @current should be replaced to the requested process.
 *   Make sure that the next process is unlinked from the @processes.
 *   If there is no process with @pid in the @processes list, fork a process
 *   from the @current. This implies the forked child process should have
 *   the identical page table entry 'values' to its parent's (i.e., @current)
 *   page table. Also, should update the writable bit properly to implement
 *   the copy-on-write feature.
 */
void switch_process(unsigned int pid) // context switch
{
	/*******************************
	 * You may switch the currently running process with switch command. 
	 * Enter the command followed by the process id to switch, and then, 
	 * the framework will call switch_process() to handle the request. 
	 * Find the target process from the processes list, and if exists, 
	 * do the context switching by replacing @current with it.
	 * If the target process does not exist, 
	 * you need to fork a child process from @current. 
	 * This implies you should allocate struct process for the child process 
	 * and initialize it (including page table) accordingly. 
	 * To duplicate the parent's address space, set up the PTE in 
	 * the child's page table to map to the same PFN of the parent. 
	 * You need to set up PTE property bits to support copy-on-write.
	*/
	bool needfork = false;
	struct process* p = NULL;

	bool init = true;

	if(list_empty(&processes) && init){
		list_add(&current->list,&processes);
		init = false;
	}


	list_for_each_entry(p,&processes,list){
		
		if(p->pid == pid){ // already exist.. don't need to fork
		
			current->pid = p->pid;
			current->pagetable = p->pagetable;
	
		}else{
			needfork = true;
		}
	}
	
	

	// fork
	if(needfork){
		// write bit 끄기
	for (int i = 0; i < NR_PTES_PER_PAGE; i++) {
	
		struct pte_directory *pd = current->pagetable.outer_ptes[i];

		if (!pd) continue;

		for (int j = 0; j < NR_PTES_PER_PAGE; j++) {
			struct pte *pte = &pd->ptes[j];
		 	pte->writable = false;
		}
	}

	
	//fork!!
		struct process* new = forked+pid;
		new->pid = pid;
		new -> pagetable = current->pagetable;
		list_add(&new->list, &processes);
		list_move(&current->list,&processes);
		//context-switch
		current = new;
		return;
	}
}

