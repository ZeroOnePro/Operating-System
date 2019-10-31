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

/* THIS FILE IS ALL YOURS; DO WHATEVER YOU WANT TO DO IN THIS FILE */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "list_head.h"

/**
 * The process which is currently running
 */
#include "process.h"
extern struct process *current;

/**
 * List head to hold the processes ready to run
 */
extern struct list_head readyqueue;


/**
 * Resources in the system.
 */
#include "resource.h"
extern struct resource resources[NR_RESOURCES];


/**
 * Monotonically increasing ticks
 */
extern unsigned int ticks;


/**
 * Quiet mode. True if the program was started with -q option
 */
extern bool quiet;

/**
 * For working PIP Protocol
 */
bool working = false;
struct process * real = NULL;
bool change = false;

/***********************************************************************
 * Default FCFS resource acquision function
 *
 * DESCRIPTION
 *   This is the default resource acquision function which is called back
 *   whenever the current process is to acquire resource @resource_id.
 *   The current implementation serves the resource in the requesting order
 *   without considering the priority. See the comments in sched.h
 ***********************************************************************/
bool fcfs_acquire(int resource_id)
{
	struct resource *r = resources + resource_id;

	if (!r->owner) {
		/* This resource is not owned by any one. Take it! */
		r->owner = current;
		return true;
	}

	/* OK, this resource is taken by @r->owner. */

	/* Update the current process state */
	current->status = PROCESS_WAIT;

	/* And append current to waitqueue */
	list_add_tail(&current->list, &r->waitqueue);

	/**
	 * And return false to indicate the resource is not available.
	 * The scheduler framework will soon call schedule() function to
	 * schedule out current and to pick the next process to run.
	 */
	return false;
}

/***********************************************************************
 * Default FCFS resource release function
 *
 * DESCRIPTION
 *   This is the default resource release function which is called back
 *   whenever the current process is to release resource @resource_id.
 *   The current implementation serves the resource in the requesting order
 *   without considering the priority. See the comments in sched.h
 ***********************************************************************/
void fcfs_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	/* Ensure that the owner process is releasing the resource */
	assert(r->owner == current);

	/* Un-own this resource */
	r->owner = NULL;

	/* Let's wake up ONE waiter (if exists) that came first */
	if (!list_empty(&r->waitqueue)) {
		struct process *waiter =
				list_first_entry(&r->waitqueue, struct process, list);

		/**
		 * Ensure the waiter  is in the wait status
		 */
		assert(waiter->status == PROCESS_WAIT);

		/**
		 * Take out the waiter from the waiting queue. Note we use
		 * list_del_init() over list_del() to maintain the list head tidy
		 * (otherwise, the framework will complain on the list head
		 * when the process exits).
		 */
		list_del_init(&waiter->list);

		/* Update the process status */
		waiter->status = PROCESS_READY;

		/**
		 * Put the waiter process into ready queue. The framework will
		 * do the rest.
		 */
		list_add_tail(&waiter->list, &readyqueue);
	}
}

/***********************************************************************
 * Priority resource acquire function
 * defualt fcfs acquire function + "prio value"  
 * acquire 1 2 4 -> 1번 리소스를 들어온 시간+2 부터 4의 시간 동안 써야한다.
 * write by seongminyoo
 **********************************************************************/
bool prio_acquire(int resource_id){

	struct resource *r = resources + resource_id;

	// this section no master for resource take it!

	if(!r->owner){
		r->owner = current;
		(r->owner)->prio_orig = (r->owner)->prio; // origin 에다 박고
		return true;
	}

	
	if(working){ // pip !
		if(current->prio > (r->owner)->prio){ 
			// r -> owner 한테 스케쥴 넘겨야 한다.
			// 상속 시켜야 된다.
			(r->owner)->prio = current->prio; // 상속
			real = current; // 얘가 자원을 놓으면 곧바로 스케쥴 되야 하므로
			change = true; // 우선순위 변화됬다고 체크
		}
	}
	
	current->status = PROCESS_WAIT; // wait상태
	list_add_tail(&current->list, &r->waitqueue); // waitqueue에 붙인다.
	return false;
}

/***********************************************************************
 * priority resource release function
 * defualt fcfs acquire function + "prio value"  
 * acquire 1 2 4 -> 1번 리소스를 들어온 시간+2 부터 4의 시간 동안 써야한다.
 * write by seongminyoo
 ***********************************************************************/
void prio_release(int resource_id)
{
	struct resource *r = resources + resource_id;

	if(change){
		// prio 원상복구
		(r->owner)->prio = (r->owner)->prio_orig;
	}

	r->owner = NULL;

	if (!list_empty(&r->waitqueue)) {
		struct process *cleaner = NULL;
		struct process * tmp = NULL;
		list_for_each_entry_safe(cleaner,tmp,&r->waitqueue,list){
			list_del_init(&cleaner->list); // 웨이트 큐에서 삭제
			cleaner->status = PROCESS_READY; // 레디 상태
			list_add(&cleaner->list,&readyqueue); // 레디큐의 첫번 째로
			list_del_init(&current->list); // 현재 스케쥴된거는 없애버리고
		}
	}

}
 
#include "sched.h"

/***********************************************************************
 * FIFO scheduler
 ***********************************************************************/
static int fifo_initialize(void)
{
	return 0;
}

static void fifo_finalize(void)
{
}

static struct process *fifo_schedule(void)
{
	struct process *next = NULL;

	/* You may inspect the situation by calling dump_status() at any time */
	// dump_status();

	/**
	 * When there was no process to run in the * previous tick (so does
	 * in the very beginning of the simulation), there will be
	 * no current process. In this case, pick the next without examining
	 * the current process. Also, when the current process is blocked
	 * while acquiring a resource, @current is (supposed to be) attached
	 * to the waitqueue of the corresponding resource. In this case just
	 * pick the next as well.
	 */
	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	/* The current process has remaining lifetime. Schedule it again */
	if (current->age < current->lifespan) {
		return current;
	}

pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)) {
		/**
		 * If the ready queue is not empty, pick the first process
		 * in the ready queue
		 */
		next = list_first_entry(&readyqueue,struct process,list);

		/**
		 * Detach the process from the ready queue. Note we use list_del_init()
		 * instead of list_del() to maintain the list head tidy. Otherwise,
		 * the framework will complain (assert) on process exit.
		 */
		list_del_init(&next->list);
	}

	/* Return the next process to run */
	return next;
}

struct scheduler fifo_scheduler = {
	.name = "FIFO",
	.acquire = fcfs_acquire,
	.release = fcfs_release,
	.initialize = fifo_initialize,
	.finalize = fifo_finalize,
	.schedule = fifo_schedule,
};


/***********************************************************************
 * SJF scheduler
 ***********************************************************************/
static struct process *sjf_schedule(void)
{
	struct process *next = NULL;
	struct process *p = NULL; // 프로세스 가리키는 변수
	struct process *sj = NULL; // Shortest job 으로 지명된 놈
	struct process *tmp = NULL;

	unsigned int __min_lifespan = 1000000; // 최대 값으로 설정하고

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	if (current->age < current->lifespan) {
		return current;
	}

pick_next:

	if (!list_empty(&readyqueue)) { // 웨이트 큐가 비지않았을 때
		
		// 리스트 순회하면서 lifespan이 제일 작은 놈을 찾아 볼것
		list_for_each_entry_safe(p,tmp,&readyqueue,list){
			if(__min_lifespan > p->lifespan){
				__min_lifespan = p->lifespan;
				sj = p;
			}
		}
		next = sj; // 이놈이 shortest job 이니까 다음번 스케쥴 대상

		list_del_init(&next->list); // 넥스트는 스케쥴 됫으니 레디큐에서 지운다.
	}
	
	return next;
}

struct scheduler sjf_scheduler = {
	.name = "Shortest-Job First",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = sjf_schedule,	/* TODO: Assign sjf_schedule()
								to this function pointer to activate
								SJF in the system */
};


/***********************************************************************
 * SRTF scheduler
 ***********************************************************************/
static struct process *srtf_schedule(void){

	struct process *next = NULL;
	struct process *p = NULL; // 프로세스 가리키는 변수
	struct process *sj = NULL; // Shortest job 으로 지명된 놈
	struct process *tmp = NULL;

	unsigned int __min_lifespan = 1000000; // 최대 값으로 설정하고

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}
	// list_move_tail -> 한 리스트에서 삭제, 다른 리스트에 추가 list_move_tail(struct list_head* list, struct list_head *head)

	if(current->age < current->lifespan){
		
		list_move(&current->list,&readyqueue);
	
	}

pick_next:

	if (!list_empty(&readyqueue)) { // 웨이트 큐가 비지 않았을 때
		// 리스트 순회하면서 lifespan이 제일 작은 놈을 찾아 볼것
		list_for_each_entry(p,&readyqueue,list){
			if(__min_lifespan > p->lifespan){
				__min_lifespan = p->lifespan;
				sj = p;
			}
			next = sj; // 이놈이 shortest job 이니까 다음번 스케쥴 대상
		}
		list_del_init(&next->list); // 넥스트는 스케쥴 됫으니 레디큐에서 지운다.
	}
	
	return next;
}

struct scheduler srtf_scheduler = {
	.name = "Shortest Remaining Time First",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = srtf_schedule,
	/* You need to check the newly created processes to implement SRTF.
	 * Use @forked() callback to mark newly created processes */
	/* Obviously, you should implement srtf_schedule() and attach it here */
};


/***********************************************************************
 * Round-robin scheduler
 ***********************************************************************/

static struct process *rr_schedule(void){

	struct process *next = NULL;

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	if (current->age < current->lifespan) {
	list_move_tail(&current->list,&readyqueue);
	}
	
pick_next:
	/* Let's pick a new process to run next */

	if (!list_empty(&readyqueue)){
		/**
		 * If the ready queue is not empty, pick the first process
		 * in the ready queue
		 */
		next = list_first_entry(&readyqueue,struct process,list);
		list_del_init(&next->list);
	}

	/* Return the next process to run */
	return next;
}
struct scheduler rr_scheduler = {
	.name = "Round-Robin",
	.acquire = fcfs_acquire, /* Use the default FCFS acquire() */
	.release = fcfs_release, /* Use the default FCFS release() */
	.schedule = rr_schedule,
	/* Obviously, you should implement rr_schedule() and attach it here */
};


/***********************************************************************
 * Priority scheduler
 ***********************************************************************/
static struct process *prio_schedule(void){

	struct process *next = NULL;
	struct process *p = NULL; // 프로세스 가리키는 변수
	struct process *doduk = NULL; // Shortest job 으로 지명된 놈
	struct process *tmp = NULL;
	
	int max_prio = -1; // 최소 값으로 설정하고

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}
	
	
	if(current->age < current->lifespan){
		list_move_tail(&current->list,&readyqueue);
	}

pick_next:

	if (!list_empty(&readyqueue)) { // 웨이트 큐가 비지 않았을 때

	
		// 리스트 순회하면서 prio 제일 큰 놈
		list_for_each_entry_safe(p,tmp,&readyqueue,list){
			if(max_prio < (int)(p->prio)){
				max_prio = (int)(p->prio);
				doduk = p;
			}
		}
		next = doduk; // 이놈이 hightest prio 이니까 다음번 스케쥴 대상
		
		list_del_init(&next->list); // 넥스트는 스케쥴 됫으니 레디큐에서 지운다.
	}
	
	return next;
}

struct scheduler prio_scheduler = {
	.name = "Priority",
	.acquire = prio_acquire, 
	.release = prio_release,
	.schedule = prio_schedule,
};


/***********************************************************************
 * Priority scheduler with priority inheritance protocol
 ***********************************************************************/
static struct process *pip_schedule(void){

	struct process *next = NULL;
	struct process *p = NULL; // 프로세스 가리키는 변수
	struct process *doduk = NULL; // Shortest job 으로 지명된 놈
	struct process *tmp = NULL;
	
	working = true;

	int max_prio = -1; // 최소 값으로 설정하고

	if (!current || current->status == PROCESS_WAIT) {
		goto pick_next;
	}

	
	if(current->age < current->lifespan){
		list_move_tail(&current->list,&readyqueue);
	}

pick_next:

	if (!list_empty(&readyqueue)) { // 웨이트 큐가 비지 않았을 때
		// 리스트 순회하면서 prio 제일 큰 놈
		list_for_each_entry_safe(p,tmp,&readyqueue,list){
			if(max_prio < (int)(p->prio)){
				max_prio = (int)(p->prio);
				doduk = p;
			}
		}
		next = doduk; // 이놈이 hightest prio 이니까 다음번 스케쥴 대상
		list_del_init(&next->list); // 넥스트는 스케쥴 됫으니 레디큐에서 지운다.
	}
	
	return next;
}

struct scheduler pip_scheduler = {
	.name = "Priority + Priority Inheritance Protocol",
	.acquire = prio_acquire, 
	.release = prio_release,
	.schedule = pip_schedule,
};
