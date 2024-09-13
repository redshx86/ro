// -------------------------------------------------------------------------------------------------
// Timer

#pragma once

#include <stdint.h>
#include "task.h"
#include "os_cfg.h"

// -------------------------------------------------------------------------------------------------

#ifdef OS_TMR

#define TMR_ELAPSE_MAX				50000

#define TMR_UNIT_TICK				0x00
#define TMR_UNIT_SECOND				0x01

// -------------------------------------------------------------------------------------------------
// One-shot timer

#ifdef OS_TMR_ONESHOT

// One-shot timer handle.
// Use tmr_oneshot_init to initialize, do not modify it directly.
// Note: Wrap into a custom structure to pass arguments to the handler.
// Note: Never modify or dispose handle of a pending timer.
struct tmr_oneshot {
	struct task_handle task;		// task.next is reused for the next timer. internal use only.
	uint16_t stamp_trig;			// next trigger timestamp. internal use only.
};

// One-shot timer static initialization.
#define TMR_ONESHOT(func) { { (void*)-1, (task_func_t)func }, 0 }

// Checks an initialized one-shot timer state.
// Returns nonzero if timer pending.
#define tmr_oneshot_pending(tmr)	((tmr)->task.next != (void*)-1)

// One-shot timer function.
typedef void (*tmr_oneshot_func_t)(struct tmr_oneshot *tmr);

// Initializes an one-shot timer handle.
// It is recommended to initialize timer handles once as a part of process initialization.
// Note: Never call for a pending timer.
void tmr_oneshot_init(struct tmr_oneshot *tmr, tmr_oneshot_func_t func);

// Starts or updates an initialized one-shot timer.
// Note: Never modify or dispose handle of a pending timer, doing so will crash the OS.
// Note: Timer function will be executed with normal priority for a tick-unit timer
//       and with low priority for a second-unit timer.
uint8_t tmr_oneshot_set(struct tmr_oneshot *tmr, uint8_t flags, uint16_t elapse);

// Cancels a pending one-shot timer.
// Retuns zero if timer not found.
// Note: This function is safe to call even if timer is not initialized.
uint8_t tmr_oneshot_cancel(struct tmr_oneshot *tmr);

#endif // OS_TMR_ONESHOT

// -------------------------------------------------------------------------------------------------
// Interval timer

#ifdef OS_TMR_INTERVAL

// Interval timer handle.
// Use tmr_interval_init to initialize, do not modify it directly.
// Note: Wrap into a custom structure to pass arguments to the timer function.
// Note: Never modify or dispose handle of an active timer.
// (Exception: interval field can be safely changed at any time).
struct tmr_interval {
	struct task_handle task;		// timer task. internal use only.
	struct tmr_interval *next;		// next timer. internal use only.
	uint16_t stamp_trig;			// next trigger timestamp. internal use only.
	uint16_t interval;				// timer interval in base units.
};

// One-shot timer static initialization.
#define TMR_INTERVAL(func, interval) { { (void*)-1, (task_func_t)func }, (void*)-1, 0, interval }

// Checks the initialized interval timer state.
// Returns nonzero if timer is active.
#define tmr_interval_active(tmr)	((tmr)->next != (void*)-1)

// Interval timer function.
typedef void (*tmr_interval_func_t)(struct tmr_interval *tmr);

// Initializes an interval timer handle.
// It is recommended to initialize timer handles once as a part of process initialization.
// Note: Never call for an active timer.
void tmr_interval_init(struct tmr_interval *tmr, tmr_interval_func_t func, uint16_t interval);

// Starts or updates an initialized interval timer.
// Note: Never modify or dispose handle of an active timer, doing so will crash the OS.
// Note: Timer function is executed with normal priority for a tick-unit timer
//       and with low priority for a second-unit timer.
uint8_t tmr_interval_set(struct tmr_interval *tmr, uint8_t flags, uint16_t elapse);

// Cancels an active interval timer (and the timer task, if scheduled).
// Retuns zero if timer not found.
// Note: This function is safe to call even if timer is not initialized.
uint8_t tmr_interval_cancel(struct tmr_interval *tmr);

#endif // OS_TMR_INTERVAL

// -------------------------------------------------------------------------------------------------
// OS internal

#ifdef OS_TMR_ONESHOT
  #ifdef OS_TMR_TICK
    #define OS_QUE_ST_TMR_ONS_TICK	0x10	// one-shot tick queue is not empty
    extern struct tmr_oneshot *tmr_ons_tick;
  #endif // OS_TMR_TICK
  #ifdef OS_TMR_SECOND
    #define OS_QUE_ST_TMR_ONS_SEC	0x20	// one-shot second queue is not empty
    extern struct tmr_oneshot *tmr_ons_sec;
  #endif // OS_TMR_SECOND
  // Schedules tasks for the one-shot timer queue until no more triggered timers.
  void tmr_ons_sched(struct tmr_oneshot **p_que, uint16_t stamp_cur, uint8_t priority);
#endif // OS_TMR_ONESHOT

#ifdef OS_TMR_INTERVAL
  #ifdef OS_TMR_TICK
    #define OS_QUE_ST_TMR_INT_TICK	0x40	// interval tick queue is not empty
    extern struct tmr_interval *tmr_int_tick;
  #endif // OS_TMR_TICK
  #ifdef OS_TMR_SECOND
    #define OS_QUE_ST_TMR_INT_SEC	0x80	// interval second queue is not empty
    extern struct tmr_interval *tmr_int_sec;
  #endif // OS_TMR_SECOND
  // Schedules task for the next interval timer.
  // Returns zero if next timer not yet triggered, or if timer task already pending.
  // Note: next timer must exist in the queue, function does not check for it.
  void tmr_int_sched(struct tmr_interval **p_queue, uint16_t stamp_cur, uint8_t priority);
#endif // OS_TMR_INTERVAL

#define OS_QUE_ST_TMR_ANY			0xF0	// any timer queue is not empty

// -------------------------------------------------------------------------------------------------

#endif // OS_TMR

// -------------------------------------------------------------------------------------------------
