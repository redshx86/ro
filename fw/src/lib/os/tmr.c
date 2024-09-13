// -------------------------------------------------------------------------------------------------
// Timer

#include <stddef.h>
#include "tmr.h"

// -------------------------------------------------------------------------------------------------

#ifdef OS_TMR

// -------------------------------------------------------------------------------------------------
// Timer data.

// Task and timer queue status bit map.
extern uint8_t os_que_st;

// Tick counters.
#ifdef OS_TMR_TICK
  extern uint16_t t_tick;
#endif // OS_TMR_TICK
#ifdef OS_TMR_SECOND
  extern uint16_t t_sec;
#endif // OS_TMR_SECOND

// -------------------------------------------------------------------------------------------------
// Timer data.

#ifdef OS_TMR_ONESHOT
  #ifdef OS_TMR_TICK
    struct tmr_oneshot *tmr_ons_tick;
  #endif // OS_TMR_TICK
  #ifdef OS_TMR_SECOND
    struct tmr_oneshot *tmr_ons_sec;
  #endif // OS_TMR_SECOND
  #if (defined OS_TMR_TICK) && (defined OS_TMR_SECOND)
    #define tmr_ons_istick(p_que)	((p_que) == &tmr_ons_tick)
  #elif (defined OS_TMR_TICK)
    #define tmr_ons_istick(p_que)	1
  #else
    #define tmr_ons_istick(p_que)	0
  #endif
#endif // OS_TMR_ONESHOT

#ifdef OS_TMR_INTERVAL
  #ifdef OS_TMR_TICK
    struct tmr_interval *tmr_int_tick;
  #endif // OS_TMR_TICK
  #ifdef OS_TMR_SECOND
    struct tmr_interval *tmr_int_sec;
  #endif // OS_TMR_SECOND
  #if (defined OS_TMR_TICK) && (defined OS_TMR_SECOND)
    #define tmr_int_istick(p_que)	((p_que) == &tmr_int_tick)
  #elif (defined OS_TMR_TICK)
    #define tmr_int_istick(p_que)	1
  #else
    #define tmr_int_istick(p_que)	0
  #endif
#endif // OS_TMR_INTERVAL

// -------------------------------------------------------------------------------------------------
// One-shot timer

#ifdef OS_TMR_ONESHOT

// Inserts one-shot timer to the queue.
// (keeps queue sorted by the next trigger timestamp)
static void tmr_ons_insert(struct tmr_oneshot **p_que,
	struct tmr_oneshot *tmr, uint16_t stamp_cur, uint16_t elapse)
{
	struct tmr_oneshot **p_ent = p_que, *ent = *p_ent;
	if(ent == NULL)
		os_que_st |= tmr_ons_istick(p_que) ? OS_QUE_ST_TMR_ONS_TICK : OS_QUE_ST_TMR_ONS_SEC;
	while(ent != NULL) {
		uint16_t ent_elap = ent->stamp_trig - stamp_cur;
		if(ent_elap > TMR_ELAPSE_MAX)
			ent_elap = 0;
		if(ent_elap > elapse)
			break;
		p_ent = (struct tmr_oneshot**) &(ent->task.next);
		ent = *p_ent;
	}
	tmr->task.next = (struct task_handle*) ent;
	*p_ent = tmr;
}

// Removes one-shot timer from the queue. Returns zero if timer not found.
static uint8_t tmr_ons_remove(struct tmr_oneshot **p_que, struct tmr_oneshot *tmr)
{
	// find the pending timer
	struct tmr_oneshot **p_ent = p_que, *ent = *p_ent;
	while(ent != tmr) {
		if(ent == NULL)
			return 0;
		p_ent = (struct tmr_oneshot**) &(ent->task.next);
		ent = *p_ent;
	}
	// remove timer from the queue
	*p_ent = (struct tmr_oneshot*) tmr->task.next;
	tmr->task.next = (void*)-1;
	if(*p_que == NULL)
		os_que_st &= tmr_ons_istick(p_que) ? ~OS_QUE_ST_TMR_ONS_TICK : ~OS_QUE_ST_TMR_ONS_SEC;
	return 1;
}

// Schedules tasks for the one-shot timer queue until no more triggered timers.
void tmr_ons_sched(struct tmr_oneshot **p_que, uint16_t stamp_cur, uint8_t priority)
{
	for(;;) {
		struct tmr_oneshot *tmr = *p_que;
		uint16_t elap;
		// exit if queue empty
		if(tmr == NULL)
			break;
		// exit if the timer not yet triggered
		elap = tmr->stamp_trig - stamp_cur;
		if((elap != 0) && (elap <= TMR_ELAPSE_MAX))
			break;
		// remove timer from the queue
		*p_que = (struct tmr_oneshot*) tmr->task.next;
		tmr->task.next = (void*)-1;
		if(*p_que == NULL)
			os_que_st &= tmr_ons_istick(p_que) ? ~OS_QUE_ST_TMR_ONS_TICK : ~OS_QUE_ST_TMR_ONS_SEC;
		// schedule the task
		task_schedule(&(tmr->task), priority);
	}
}

// Initializes an one-shot timer handle.
// It is recommended to initialize timer handles once as a part of process initialization.
// Note: Never call for a pending timer.
void tmr_oneshot_init(struct tmr_oneshot *tmr, tmr_oneshot_func_t func)
{
	tmr->task.next = (void*)-1;
	tmr->task.func = (task_func_t) func;
}

// Starts or updates an initialized one-shot timer.
// Note: Never modify or dispose handle of a pending timer, doing so will crash the OS.
// Note: Timer function will be executed with normal priority for a tick-unit timer
//       and with low priority for a second-unit timer.
uint8_t tmr_oneshot_set(struct tmr_oneshot *tmr, uint8_t flags, uint16_t elapse)
{
	struct tmr_oneshot **p_que;
	uint16_t stamp_cur;
	// check parameters
#ifdef OS_PARAM_CHECK
	if((tmr == NULL) || (tmr->task.func == NULL) || (elapse > TMR_ELAPSE_MAX))
		return 0;
#endif // OS_PARAM_CHECK
	// cancel the pending timer
	if(tmr_oneshot_pending(tmr) && !tmr_oneshot_cancel(tmr))
		return 0;
	// select the time unit and timer queue
	if(flags & TMR_UNIT_SECOND) {
#ifdef OS_TMR_SECOND
		p_que = &tmr_ons_sec;
		stamp_cur = t_sec;
#else // !OS_TMR_SECOND
		return 0;
#endif // !OS_TMR_SECOND
	} else {
#ifdef OS_TMR_TICK
		p_que = &tmr_ons_tick;
		stamp_cur = t_tick;
#else // !OS_TMR_TICK
		return 0;
#endif // !OS_TMR_TICK
	}
	// set the trigger timestamp
	tmr->stamp_trig = stamp_cur + elapse;
	// insert timer to the queue
	tmr_ons_insert(p_que, tmr, stamp_cur, elapse);
	return 1;
}

// Cancels a pending one-shot timer.
// Retuns zero if timer not found.
// Note: This function is safe to call even if timer is not initialized.
uint8_t tmr_oneshot_cancel(struct tmr_oneshot *tmr)
{
#ifdef OS_PARAM_CHECK
	if(tmr == NULL)
		return 0;
#endif // OS_PARAM_CHECK
	if(!(tmr_oneshot_pending(tmr)))
		return 0;
#ifdef OS_TMR_TICK
	if((tmr_ons_tick != NULL) && tmr_ons_remove(&tmr_ons_tick, tmr))
		return 1;
#endif // OS_TMR_TICK
#ifdef OS_TMR_SECOND
	if((tmr_ons_sec != NULL) && tmr_ons_remove(&tmr_ons_sec, tmr))
		return 1;
#endif // OS_TMR_SECOND
	return task_cancel(&(tmr->task));
}

#endif // OS_TMR_ONESHOT

// -------------------------------------------------------------------------------------------------
// Interval timer

#ifdef OS_TMR_INTERVAL

// Inserts interval timer to the queue.
// (keeps queue sorted by the next trigger timestamp)
static void tmr_int_insert(struct tmr_interval **p_que,
	struct tmr_interval *tmr, uint16_t stamp_cur, uint16_t elapse)
{
	struct tmr_interval **p_ent = p_que, *ent = *p_ent;
	if(ent == NULL)
		os_que_st |= tmr_int_istick(p_que) ? OS_QUE_ST_TMR_INT_TICK : OS_QUE_ST_TMR_INT_SEC;
	while(ent != NULL) {
		uint16_t ent_elap = ent->stamp_trig - stamp_cur;
		if(ent_elap > TMR_ELAPSE_MAX)
			ent_elap = 0;
		if(ent_elap > elapse)
			break;
		p_ent = &(ent->next);
		ent = *p_ent;
	}
	tmr->next = ent;
	*p_ent = tmr;
}

// Removes interval timer from the queue. Returns zero if timer not found.
static uint8_t tmr_int_remove(struct tmr_interval **p_que, struct tmr_interval *tmr)
{
	// find the timer
	struct tmr_interval **p_ent = p_que, *ent = *p_ent;
	while(ent != tmr) {
		if(ent == NULL)
			return 0;
		p_ent = &(ent->next);
		ent = *p_ent;
	}
	// remove timer from the queue
	*p_ent = tmr->next;
	tmr->next = (void*)-1;
	if(*p_que == NULL)
		os_que_st &= tmr_int_istick(p_que) ? ~OS_QUE_ST_TMR_INT_TICK : ~OS_QUE_ST_TMR_INT_SEC;
	return 1;
}

// Schedules tasks for the interval timer queue until no more triggered timers.
void tmr_int_sched(struct tmr_interval **p_que, uint16_t stamp_cur, uint8_t priority)
{
	for(;;) {
		struct tmr_interval *tmr = *p_que;
		uint16_t elap;
		// exit if queue empty or if next timer task pending (we need to wait for it to execute)
		if((tmr == NULL) || task_pending(&(tmr->task)))
			break;
		// exit if the timer not yet triggered
		elap = tmr->stamp_trig - stamp_cur;
		if((elap != 0) && (elap <= TMR_ELAPSE_MAX))
			break;
		// remove timer from the queue
		*p_que = tmr->next;
		tmr->next = (void*)-1;
		if(*p_que == NULL)
			os_que_st &= tmr_int_istick(p_que) ? ~OS_QUE_ST_TMR_INT_TICK : ~OS_QUE_ST_TMR_INT_SEC;
		// restart the timer if interval is nonzero
		if(tmr->interval != 0) {
			// update the trigger timestamp
			// slip the trigger if the updated timestamp already elapsed
			tmr->stamp_trig += tmr->interval;
			elap += tmr->interval;
			if(elap > TMR_ELAPSE_MAX) {
				tmr->stamp_trig = stamp_cur;
				elap = 0;
			}
			// insert timer back to the queue
			tmr_int_insert(p_que, tmr, stamp_cur, elap);
		}
		// schedule the task
		task_schedule(&(tmr->task), priority);
	}
}

// Initializes an interval timer handle.
// It is recommended to initialize timer handles once as a part of process initialization.
// Note: Never call for an active timer.
void tmr_interval_init(struct tmr_interval *tmr, tmr_interval_func_t func, uint16_t interval)
{
	tmr->task.next = (void*)-1;
	tmr->task.func = (task_func_t) func;
	tmr->next = (void*)-1;
	tmr->interval = interval;
}

// Starts or updates an initialized interval timer.
// Note: Never modify or dispose handle of an active timer, doing so will crash the OS.
// Note: Timer function is executed with normal priority for a tick-unit timer
//       and with low priority for a second-unit timer.
uint8_t tmr_interval_set(struct tmr_interval *tmr, uint8_t flags, uint16_t elapse)
{
	struct tmr_interval **p_que;
	uint16_t stamp_cur;

#ifdef OS_PARAM_CHECK
	if( (tmr == NULL) || (tmr->task.func == NULL) || (elapse > TMR_ELAPSE_MAX) ||
		(tmr->interval > TMR_ELAPSE_MAX) )
	{
		return 0;
	}
#endif // OS_PARAM_CHECK
	// cancel the active timer, cancel the pending timer task
	if(tmr_interval_active(tmr) || task_pending(&(tmr->task))) {
		tmr_interval_cancel(tmr);
		if(tmr_interval_active(tmr) || task_pending(&(tmr->task)))
			return 0;
	}
	// select the time unit and timer queue
	if(flags & TMR_UNIT_SECOND) {
#ifdef OS_TMR_SECOND
		p_que = &tmr_int_sec;
		stamp_cur = t_sec;
#else // !OS_TMR_SECOND
		return 0;
#endif // !OS_TMR_SECOND
	} else {
#ifdef OS_TMR_TICK
		p_que = &tmr_int_tick;
		stamp_cur = t_tick;
#else // !OS_TMR_TICK
		return 0;
#endif // !OS_TMR_TICK
	}
	// set the trigger timestamp
	tmr->stamp_trig = stamp_cur + elapse;
	// insert timer to the queue
	tmr_int_insert(p_que, tmr, stamp_cur, elapse);
	return 1;
}

// Cancels an active interval timer (and the timer task, if scheduled).
// Retuns zero if timer not found.
// Note: This function is safe to call even if timer is not initialized.
uint8_t tmr_interval_cancel(struct tmr_interval *tmr)
{
	uint8_t result = 0;
#ifdef OS_PARAM_CHECK
	if(tmr == NULL)
		return 0;
#endif // OS_PARAM_CHECK
#ifdef OS_TMR_TICK
	if((tmr_int_tick != NULL) && tmr_int_remove(&tmr_int_tick, tmr))
		result = 1;
#endif // OS_TMR_TICK
#ifdef OS_TMR_SECOND
	if(!result && (tmr_int_sec != NULL) && tmr_int_remove(&tmr_int_sec, tmr))
		result = 1;
#endif // OS_TMR_SECOND
	if(task_pending(&(tmr->task)) && task_cancel(&(tmr->task)))
		result = 1;
	return result;
}

#endif // OS_TMR_INTERVAL

// -------------------------------------------------------------------------------------------------

#endif // OS_TMR

// -------------------------------------------------------------------------------------------------
