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
#include <string.h>
#include <stdlib.h>

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

// make process

struct process* forked[100];
struct pagetable pagetable[100];

/****************************************************************************/
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

	forked[current->pid] = current;
	
	if(!forked[current->pid]->pagetable.outer_ptes[outer_index]){ // initial
		 return false;
	}else if(!forked[current->pid]->pagetable.outer_ptes[outer_index]->ptes[inner_index].valid){ // inner page table valid = false
		return false;
	}else{
		*pfn = forked[current->pid]->pagetable.outer_ptes[outer_index]->ptes[inner_index].pfn;
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

	// make first page table
	
	// init inner pagetable

	forked[current->pid] = current;
	// p0
	if(!forked[current->pid]->pagetable.outer_ptes[outer_index]){
		//printf("::no outer::\n");
		struct pte_directory pd[NR_PTES_PER_PAGE*10];
		
		struct pte p[NR_PTES_PER_PAGE*10];
		
		// pte에 pfn값 할당
		p[inner_index].pfn = alloc_page();
		// pte의 valid bit 변경
		p[inner_index].valid = true;
		// pte의 writable 변경
		p[inner_index].writable = true;
		// inner page table 로 집어넣고
		pd[outer_index].ptes[inner_index].pfn = p[inner_index].pfn;
		pd[outer_index].ptes[inner_index].valid = p[inner_index].valid;
		pd[outer_index].ptes[inner_index].writable = p[inner_index].writable;
		// outer page table 로 집어넣고
		struct pte_directory* pdp1 = &pd[outer_index];
		struct pte_directory* pdp2 = malloc(sizeof(struct pte_directory*));
		memcpy(&pdp2,&pdp1,sizeof(struct pte_directory*));
		// process와 연결 지어주고
		pagetable[current->pid].outer_ptes[outer_index] = pdp2;
		forked[current->pid]->pagetable = pagetable[current->pid];
		//memset(&pdp1,0,sizeof(struct pte_directory*));
		//memset(&pdp2,0,sizeof(struct pte_directory*));
	}else if(!forked[current->pid]->pagetable.outer_ptes[outer_index]->ptes[inner_index].valid){
		//printf("::not valid::\n");
	
		// outer랑 inner는 있으므로 pte와 연결한다.
		forked[current->pid]->pagetable.outer_ptes[outer_index]->ptes[inner_index].pfn = alloc_page();
		forked[current->pid]->pagetable.outer_ptes[outer_index]->ptes[inner_index].valid = true;
		forked[current->pid]->pagetable.outer_ptes[outer_index]->ptes[inner_index].writable = true;
	}
	// 생성한 pte와 current 의 inner page table의 pte를 맵
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

bool isforked[4] = {true, false, false, false};

void switch_process(unsigned int pid) // context switch
{
	// init process list에 추가
	struct process* p = NULL;

	bool init = true;

	if(list_empty(&processes) && init){
		list_add(&current->list,&processes);
		init = false;
	}

	// case1 : fork 해야하는 경우
	// 처음 온 프로세스 플래그
	

	if(!isforked[pid]){
		// fork 됬음으로 바꿈
		isforked[pid] = true;
		// current 의 writable bit를 끈다.
		for (int i = 0; i < NR_PTES_PER_PAGE; i++) {
	
			struct pte_directory *pd = current->pagetable.outer_ptes[i];

			if (!pd) continue;

			for (int j = 0; j < NR_PTES_PER_PAGE; j++) {
				struct pte *pte = &pd->ptes[j];
		 		pte->writable = false;
			}
		}
		// fork -> pid change, make new pagetable and copy current's pagetable
		
		struct process * new = malloc(sizeof(struct process *)); 

		struct pte_directory pd[NR_PTES_PER_PAGE*10];
		
		struct pte p[NR_PTES_PER_PAGE*10];

		
	}

	// case2 : list 뒤져서 바꿔주기만 하면 되는 경우
}

