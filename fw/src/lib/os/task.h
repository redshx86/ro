// -------------------------------------------------------------------------------------------------
// Task

#pragma once

#include <stdint.h>
#include "os_cfg.h"

// -------------------------------------------------------------------------------------------------
// Task

#define TASK_PRIORITY_LOW			0x00
#define TASK_PRIORITY_NORMAL		0x01
#define TASK_PRIORITY_HIGH			0x02

// Task function.
struct task_handle;
typedef void (*task_func_t)(struct task_handle *task);

// Task handle.
// Use task_init to initialize a task, do not change structure manually.
// Note: Wrap into a custom structure to pass arguments to the task function.
// Note: Never modify or dispose handle of a pending task.
struct task_handle {
	struct task_handle *next;		// next item. internal use only.
	task_func_t func;				// task function.
};

// Task handle static initialization.
#define TASK_HANDLE(func)			{ (void*)-1, func }

// Checks the initialized task handle state.
// Returns nonzero if task pending.
#define task_pending(task)			((task)->next != (void*)-1)

// Initializes a task handle.
// It is recommended to initialize task handles once as a part of process initialization.
// Note: Never call for a pending task.
void task_init(struct task_handle *task, task_func_t func);

// Schedules a task.
// Returns zero if task already scheduled.
// Note: Task handle must be initialized.
// Note: Never modify or dispose the handle for a pending task, doing so will crash the OS.
uint8_t task_schedule(struct task_handle *task, uint8_t priority);

// Cancels a pending task.
// Returns zero if task not found.
// Note: This function is safe to call even if the task handle is not initialized.
uint8_t task_cancel(struct task_handle *task);

// -------------------------------------------------------------------------------------------------
// Flag-triggered task

// Binds initialized handle to the task trigger flag.
// Returns zero if the specified flag already used.
// Note: Never modify or dispose the passed task handle before unbinding it from the flag.
// Note: Flag-triggered task priority must be normal or high.
uint8_t task_flag_bind(uint8_t i, struct task_handle *task, uint8_t priority);

// Unbinds task from the task trigger flag.
// Returns zero if no task bound to the specified flag.
uint8_t task_flag_unbind(uint8_t i);

// -------------------------------------------------------------------------------------------------
// Dynamic task pool

#ifdef OS_TASK_DYN_COUNT

// Dynamic task function.
typedef void (*dynamic_task_func_t)(struct task_handle *task, void *ctx);

// Allocates and initializes a dynamic task handle.
// Returns NULL if no dynamic task handle available.
struct task_handle * task_alloc(dynamic_task_func_t func, void *ctx);

// Frees a dynamic task handle.
// Note: Pending task will be automatically cancelled.
// Note: Handle will be automatically unbound from the task trigger flag.
uint8_t task_free(struct task_handle *task);

#endif // OS_TASK_DYN_COUNT

// -------------------------------------------------------------------------------------------------
// OS internal

// Queue state flags.
#define OS_QUE_ST_TASK_LOW			(1<<TASK_PRIORITY_LOW)		// low priority queue not empty
#define OS_QUE_ST_TASK_NORMAL		(1<<TASK_PRIORITY_NORMAL)	// normal priority queue not empty
#define OS_QUE_ST_TASK_HIGH			(1<<TASK_PRIORITY_HIGH)		// high priority queue not empty
#define OS_QUE_ST_TASK_ANY			(OS_QUE_ST_TASK_LOW|OS_QUE_ST_TASK_NORMAL|OS_QUE_ST_TASK_HIGH)

// Initializes the task data.
void task_data_init();

// Schedules the flag-triggered tasks.
void task_sched_flags(uint8_t pos, uint8_t fl);

// Executes the next scheduled task with the highest priority.
// Returns zero if all task queues are empty.
uint8_t task_run_next();

// -------------------------------------------------------------------------------------------------
