// -------------------------------------------------------------------------------------------------

#include <stddef.h>
#include "task.h"

// -------------------------------------------------------------------------------------------------
// Global task data.

// Task and timer queue status bit map.
extern uint8_t os_que_st;

// Flag-triggered task registers.
#ifndef OS_TASK_REG_0
uint8_t volatile task_reg_0;
#endif // !OS_TASK_REG_0
#if (OS_TASK_FLAG_COUNT > 8) && (!defined OS_TASK_REG_1)
uint8_t volatile task_reg_1;
#endif // !OS_TASK_REG_1
#if (OS_TASK_FLAG_COUNT > 16) && (!defined OS_TASK_REG_2)
uint8_t volatile task_reg_2;
#endif // !OS_TASK_REG_2
#if (OS_TASK_FLAG_COUNT > 24) && (!defined OS_TASK_REG_3)
uint8_t volatile task_reg_3;
#endif // !OS_TASK_REG_3

// -------------------------------------------------------------------------------------------------
// Task data (private).

// Task queues, one per priority level.
#define TASK_QUEUE_COUNT			(TASK_PRIORITY_HIGH + 1)
static struct task_handle *task_next[TASK_QUEUE_COUNT];
static struct task_handle **p_task_new[TASK_QUEUE_COUNT];

// Flag-triggered task table.
static struct task_handle *task_flag_table[OS_TASK_FLAG_COUNT];
static uint8_t task_flag_priority[(OS_TASK_FLAG_COUNT + 7) / 8];

// Dynamic task handle pool.
#ifdef OS_TASK_DYN_COUNT
struct dyn_task_handle {
	struct task_handle task;
	void *ctx;
};
static struct dyn_task_handle dyn_task_pool[OS_TASK_DYN_COUNT];
static struct dyn_task_handle *dyn_task_freelist;
#endif // OS_TASK_DYN_COUNT

// -------------------------------------------------------------------------------------------------
// Task

// Initializes a task handle.
// It is recommended to initialize task handles once as a part of process initialization.
// Note: Never call for a pending task.
void task_init(struct task_handle *task, task_func_t func)
{
	task->next = (void*)-1;
	task->func = func;
}

// Schedules a task.
// Returns zero if task already scheduled.
// Note: Task handle must be initialized.
// Note: Never modify or dispose the handle for a pending task, doing so will crash the OS.
uint8_t task_schedule(struct task_handle *task, uint8_t priority)
{
#ifdef OS_PARAM_CHECK
	// check parameters
	if((task == NULL) || (task->func == NULL) || (priority >= TASK_QUEUE_COUNT))
		return 0;
#endif
	// fail if task pending
	if(task_pending(task))
		return 0;
	// append task to the queue
	task->next = *(p_task_new[priority]);
	*(p_task_new[priority]) = task;
	p_task_new[priority] = &(task->next);
	os_que_st |= 1<<priority;
	return 1;
}

// Cancels a pending task.
// Returns zero if task not found.
// Note: This function is safe to call even if the task handle is not initialized.
uint8_t task_cancel(struct task_handle *task)
{
	uint8_t pr;
	struct task_handle **p_ent, *ent;
#ifdef OS_PARAM_CHECK
	if(task == NULL)
		return 0;
#endif // OS_PARAM_CHECK
	// cancel the pending task
	if(!task_pending(task))
		return 0;
	// traverse task queues to find the task
	for(pr = 0; pr < TASK_QUEUE_COUNT; pr++) {
		p_ent = &(task_next[pr]);
		ent = *p_ent;
		while(ent != NULL) {
			if(ent == task)
				break;
			p_ent = &(ent->next);
			ent = *p_ent;
		}
		if(ent != NULL)
			break;
	}
	if(ent == NULL)
		return 0;
	// remove task from the queue
	if(task->next == NULL)
		p_task_new[pr] = p_ent;
	*p_ent = task->next;
	task->next = (void*)-1;
	if(task_next[pr] == NULL)
		os_que_st &= ~(1<<pr);
	return 1;
}

// Executes the next scheduled task with the highest priority.
// Returns zero if all task queues are empty.
uint8_t task_run_next()
{
	// fetch next task from the queue
	// don't use loop for the faster code
	struct task_handle *task;
	if(os_que_st & (1<<TASK_PRIORITY_HIGH)) {
		task = task_next[TASK_PRIORITY_HIGH];
		task_next[TASK_PRIORITY_HIGH] = task->next;
		if(task_next[TASK_PRIORITY_HIGH] == NULL) {
			p_task_new[TASK_PRIORITY_HIGH] = &(task_next[TASK_PRIORITY_HIGH]);
			os_que_st &= ~(1<<TASK_PRIORITY_HIGH);
		}
	} else if(os_que_st & (1<<TASK_PRIORITY_NORMAL)) {
		task = task_next[TASK_PRIORITY_NORMAL];
		task_next[TASK_PRIORITY_NORMAL] = task->next;
		if(task_next[TASK_PRIORITY_NORMAL] == NULL) {
			p_task_new[TASK_PRIORITY_NORMAL] = &(task_next[TASK_PRIORITY_NORMAL]);
			os_que_st &= ~(1<<TASK_PRIORITY_NORMAL);
		}
	} else if(os_que_st & (1<<TASK_PRIORITY_LOW)) {
		task = task_next[TASK_PRIORITY_LOW];
		task_next[TASK_PRIORITY_LOW] = task->next;
		if(task_next[TASK_PRIORITY_LOW] == NULL) {
			p_task_new[TASK_PRIORITY_LOW] = &(task_next[TASK_PRIORITY_LOW]);
			os_que_st &= ~(1<<TASK_PRIORITY_LOW);
		}
	} else {
		return 0;
	}
	task->next = (void*)-1;
	// call the task function
#ifdef OS_TASK_DYN_COUNT
	if( (task >= (struct task_handle*) dyn_task_pool) &&
		(task < (struct task_handle*) (dyn_task_pool + OS_TASK_DYN_COUNT)) )
	{
		struct dyn_task_handle *dyn_task = (struct dyn_task_handle *) task;
		dynamic_task_func_t func = (dynamic_task_func_t) dyn_task->task.func;
		func(task, dyn_task->ctx);
	}
	else
	{
#endif // OS_TASK_DYN_COUNT
		task->func(task);
#ifdef OS_TASK_DYN_COUNT
	}
#endif // OS_TASK_DYN_COUNT
	return 1;
}

// -------------------------------------------------------------------------------------------------
// Flag-triggered task

// Binds initialized handle to the task trigger flag.
// Returns zero if the specified flag already used.
// Note: Never modify or dispose the passed task handle before unbinding it from the flag.
// Note: Flag-triggered task priority must be normal or high.
uint8_t task_flag_bind(uint8_t i, struct task_handle *task, uint8_t priority)
{
#ifdef OS_PARAM_CHECK
	if( (i >= OS_TASK_FLAG_COUNT) || (task == NULL) || (task->func == NULL) ||
		(priority < TASK_PRIORITY_NORMAL) || (priority > TASK_PRIORITY_HIGH) )
	{
		return 0;
	}
#endif // OS_PARAM_CHECK
	if((task_flag_table[i] != NULL) || task_pending(task))
		return 0;
	task_flag_table[i] = task;
	if(priority == TASK_PRIORITY_HIGH)
		task_flag_priority[i >> 3] |= 1 << (i & 0x7);
	return 1;
}

// Unbinds task from the task trigger flag.
// Returns zero if no task bound to the specified flag.
uint8_t task_flag_unbind(uint8_t i)
{
#ifdef OS_PARAM_CHECK
	if(i >= OS_TASK_FLAG_COUNT)
		return 0;
#endif // OS_PARAM_CHECK
	if(task_flag_table[i] == NULL)
		return 0;
	task_cancel(task_flag_table[i]);
	task_flag_table[i] = NULL;
	task_flag_priority[i >> 3] &= ~(1 << (i & 0x7));
	return 1;
}

// Schedules the flag-triggered tasks.
void task_sched_flags(uint8_t pos, uint8_t fl)
{
	while(fl != 0) {
		// priority select the next flag
		uint8_t i, m;
		if(fl & 0x0f) {
			if(fl & 0x03) { if(fl & 0x01) { i = 0; m = 0x01; } else { i = 1; m = 0x02; } }
			else          { if(fl & 0x04) { i = 2; m = 0x04; } else { i = 3; m = 0x08; } }
		} else {
			if(fl & 0x30) { if(fl & 0x10) { i = 4; m = 0x10; } else { i = 5; m = 0x20; } }
			else          { if(fl & 0x40) { i = 6; m = 0x40; } else { i = 7; m = 0x80; } }
		}
		fl &= ~m;
		i += pos;
		// schedule the task
		if((i < OS_TASK_FLAG_COUNT) && (task_flag_table[i] != NULL)) {
			task_schedule(task_flag_table[i],
				(task_flag_priority[i >> 3] & m) ? TASK_PRIORITY_HIGH : TASK_PRIORITY_NORMAL);
		}
	}
}

// -------------------------------------------------------------------------------------------------
// Dynamic task pool

#ifdef OS_TASK_DYN_COUNT

// Allocates and initializes a dynamic task handle.
// Returns NULL if no dynamic task handle available.
struct task_handle * task_alloc(dynamic_task_func_t func, void *ctx)
{
	struct dyn_task_handle *dyn_task;
#ifdef OS_PARAM_CHECK
	if(func == NULL)
		return NULL;
#endif // OS_PARAM_CHECK
	// grab an entry from the dynmamic task freelist
	if(dyn_task_freelist == NULL)
		return NULL;
	dyn_task = dyn_task_freelist;
	dyn_task_freelist = (struct dyn_task_handle*) dyn_task->task.next;
	// initialize the dynamic task
	dyn_task->task.func = (task_func_t) func;
	dyn_task->ctx = ctx;
	return (struct task_handle *) dyn_task;
}

// Frees a dynamic task handle.
// Note: Pending task will be automatically cancelled.
// Note: Handle will be automatically unbound from the task trigger flag.
uint8_t task_free(struct task_handle *task)
{
	struct dyn_task_handle *dyn_task = (struct dyn_task_handle*) task;
	uint8_t i;
#ifdef OS_PARAM_CHECK
	if((dyn_task < dyn_task_pool) || (dyn_task >= dyn_task_pool + OS_TASK_DYN_COUNT))
		return 0;
#endif // OS_PARAM_CHECK
	// cancel the pending task
	task_cancel((struct task_handle*)dyn_task);
	// remove the flag-triggered task table entry
	for(i = 0; i < OS_TASK_FLAG_COUNT; i++) {
		if(task_flag_table[i] == (struct task_handle*)dyn_task)
			task_flag_unbind(i);
	}
	// return dynamic entry to the freelist
	dyn_task->task.next = (struct task_handle*) dyn_task_freelist;
	dyn_task_freelist = dyn_task;
	return 1;
}

#endif // OS_TASK_DYN_COUNT

// -------------------------------------------------------------------------------------------------
// Init

// Initializes the task data.
void task_data_init()
{
	uint8_t i;
	for(i = 0; i < TASK_QUEUE_COUNT; i++)
		p_task_new[i] = &(task_next[i]);
#ifdef OS_TASK_DYN_COUNT
	for(i = 0; i < OS_TASK_DYN_COUNT - 1; i++)
		dyn_task_pool[i].task.next = &(dyn_task_pool[i + 1].task);
	dyn_task_freelist = &(dyn_task_pool[0]);
#endif // OS_TASK_DYN_COUNT
}

// -------------------------------------------------------------------------------------------------
