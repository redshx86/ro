// -------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include "os.h"

// -------------------------------------------------------------------------------------------------
// OS global data.

// Task and timer queue status bit map.
uint8_t os_que_st;

// -------------------------------------------------------------------------------------------------
// Stack checking.

#ifdef OS_STK_SIZE

#define STACK_END			(RAMEND + 1)
#define STACK_START			(STACK_END - OS_STK_SIZE)
#define STACK_COOKIE		0x35CA
#define STACK_FILL			0x5A

// Initialize stack size and fill the stack.
static void stk_init()
{
#ifdef OS_STK_MAX_USAGE
	uint16_t p, p_end = SP;
	for(p = STACK_START; p <= p_end; p++)
		*(uint8_t*)p = STACK_FILL;
#endif // OS_STK_MAX_USAGE
	*(uint16_t*)(STACK_START - 2) = STACK_COOKIE;
#ifdef OS_STK_ADJUST_HEAP
	__malloc_heap_end = (void*)(STACK_START - 2);
#endif // OS_STK_ADJUST_HEAP
}

// Checks if stack cookie was damaged.
void stk_check_cookie()
{
	if(*(uint16_t*)(STACK_START - 2) != STACK_COOKIE) {
		cli();
#ifdef STK_OVF_ACTION
		STK_OVF_ACTION();
#endif // STK_OVF_ACTION
		for(;;)
			;
	}
}

// Assesses the max stack usage by checking how much stack bytes were modified since the start.
#ifdef OS_STK_MAX_USAGE
uint16_t stk_max_usage()
{
	uint16_t p;
	for(p = STACK_START; p != STACK_END; p++) {
		if(*(uint8_t*)p != STACK_FILL)
			break;
	}
	return STACK_END - p;
}
#endif // OS_STK_MAX_USAGE

#endif // OS_STK_SIZE

// -------------------------------------------------------------------------------------------------
// Tick counters.

// Tick source counter updated by the user interrupt.
uint16_t volatile t_tick_src;

// Tick update task.
static void tick_upd(struct task_handle *_task);
static struct task_handle tick_upd_task = TASK_HANDLE(tick_upd);

// Tick counter.
uint16_t t_tick;

// Seconds counter.
#ifdef OS_TICK_SEC_DIV_INT
  uint16_t t_sec;
  static uint16_t t_sec_prev;
  #if (defined OS_TICK_SEC_DIV_P) && (defined OS_TICK_SEC_DIV_Q)
    #if (OS_TICK_SEC_DIV_P + OS_TICK_SEC_DIV_Q < 256)
      typedef uint8_t sec_err_t;
    #else
      typedef uint16_t sec_err_t;
    #endif
    static sec_err_t t_sec_err;
  #endif // (defined OS_TICK_SEC_DIV_P) && (defined OS_TICK_SEC_DIV_Q)
#endif // OS_TICK_SEC_DIV_INT

// Tick update task function.
static void tick_upd(struct task_handle *_task)
{
#ifdef OS_TMR
	uint8_t qs = os_que_st;
#endif // OS_TMR

	uint16_t tsrc;

	// -----------------------------------------------------
	// Tick update.

	// Update the tick counter
	ATOMIC_BLOCK(ATOMIC_FORCEON) { tsrc = t_tick_src; }
	t_tick = tsrc;

	// Schedule the tick timer tasks.
#if (defined OS_TMR) && (defined OS_TMR_TICK)
  #ifdef OS_TMR_ONESHOT
	if(qs & OS_QUE_ST_TMR_ONS_TICK)
		tmr_ons_sched(&tmr_ons_tick, tsrc, TASK_PRIORITY_NORMAL);
  #endif // OS_TMR_ONESHOT
  #ifdef OS_TMR_INTERVAL
	if(qs & OS_QUE_ST_TMR_INT_TICK)
		tmr_int_sched(&tmr_int_tick, tsrc, TASK_PRIORITY_NORMAL);
  #endif // OS_TMR_INTERVAL
#endif // OS_TMR_TICK

	// -----------------------------------------------------
	// Seconds update.

	// Update the second counter.
#ifdef OS_TICK_SEC_DIV_INT
  #if !((defined OS_TICK_SEC_DIV_P) && (defined OS_TICK_SEC_DIV_Q))
	if(tsrc - t_sec_prev >= OS_TICK_SEC_DIV_INT) {
		uint16_t sec_cnt = t_sec;
		uint16_t sec_prev = t_sec_prev;
		uint16_t elap;
		do {
			sec_cnt++;
			sec_prev += OS_TICK_SEC_DIV_INT;
			elap = tsrc - sec_prev;
//		} while(tsrc - sec_prev >= OS_TICK_SEC_DIV_INT);
		} while((elap >= OS_TICK_SEC_DIV_INT) && (elap < 0xffff)); // prevent gcc "optimization"
		t_sec = sec_cnt;
		t_sec_prev = sec_prev;
  #else // (defined OS_TICK_SEC_DIV_P) && (defined OS_TICK_SEC_DIV_Q)
	if(tsrc - t_sec_prev >= OS_TICK_SEC_DIV_INT + 1) {
		uint16_t sec_cnt = t_sec;
		uint16_t sec_prev = t_sec_prev;
		sec_err_t sec_err = t_sec_err;
		do {
			sec_cnt++;
			sec_prev += OS_TICK_SEC_DIV_INT + 1;
			sec_err += OS_TICK_SEC_DIV_Q - OS_TICK_SEC_DIV_P;
			if(sec_err >= OS_TICK_SEC_DIV_Q) {
				sec_err -= OS_TICK_SEC_DIV_Q;
				sec_prev--;
			}
		} while(tsrc - sec_prev >= OS_TICK_SEC_DIV_INT + 1);
		t_sec = sec_cnt;
		t_sec_prev = sec_prev;
		t_sec_err = sec_err;
  #endif // (defined OS_TICK_SEC_DIV_P) && (defined OS_TICK_SEC_DIV_Q)

	// Schedule the second timer tasks.
#if (defined OS_TMR) && (defined OS_TMR_SECOND)
  #ifdef OS_TMR_ONESHOT
		if(qs & OS_QUE_ST_TMR_ONS_SEC)
			tmr_ons_sched(&tmr_ons_sec, sec_cnt, TASK_PRIORITY_LOW);
  #endif // OS_TMR_ONESHOT
  #ifdef OS_TMR_INTERVAL
		if(qs & OS_QUE_ST_TMR_INT_SEC)
			tmr_int_sched(&tmr_int_sec, sec_cnt, TASK_PRIORITY_LOW);
  #endif // OS_TMR_INTERVAL
#endif // OS_TMR_SECOND

	}
#endif // OS_TICK_SEC_DIV_INT
}

#ifdef OS_TICK_READ
uint16_t t_tick_read()
{
	uint16_t tsrc;
	ATOMIC_BLOCK(ATOMIC_FORCEON) { tsrc = t_tick_src; }
	return tsrc;
}
#endif // OS_TICK_READ

#ifdef OS_TICK_WAIT
void t_tick_wait(uint16_t n)
{
	uint16_t tsrc_ofs, tsrc;
	ATOMIC_BLOCK(ATOMIC_FORCEON) { tsrc_ofs = t_tick_src; }
	do {
		ATOMIC_BLOCK(ATOMIC_FORCEON) { tsrc = t_tick_src; }
	} while(tsrc - tsrc_ofs < n);
}
#endif // OS_TICK_WAIT

// -------------------------------------------------------------------------------------------------
// Task flag checking.
// This task-related code is implemented in the main OS module for better embedding in the OS loop.

// Check if any task flag set.
// Returns zero if no task flags are set.
static uint8_t task_flag_check()
{
	return (
		OS_TASK_REG_0
#if (OS_TASK_FLAG_COUNT > 8)
		| OS_TASK_REG_1
#endif // OS_TASK_FLAG_COUNT > 8
#if (OS_TASK_FLAG_COUNT > 16)
		| OS_TASK_REG_2
#endif // OS_TASK_FLAG_COUNT > 16
#if (OS_TASK_FLAG_COUNT > 24)
		| OS_TASK_REG_3
#endif // OS_TASK_FLAG_COUNT > 24
	);
}

// Schedule tasks for the set flags.
// Reset task flags.
static void task_flag_sched_all()
{
	uint8_t f0;
#if (OS_TASK_FLAG_COUNT > 8)
	uint8_t f1;
#endif // OS_TASK_FLAG_COUNT > 8
#if (OS_TASK_FLAG_COUNT > 16)
	uint8_t f2;
#endif // OS_TASK_FLAG_COUNT > 16
#if (OS_TASK_FLAG_COUNT > 24)
	uint8_t f3;
#endif // OS_TASK_FLAG_COUNT > 24
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		f0 = OS_TASK_REG_0;
		OS_TASK_REG_0 = 0;
#if (OS_TASK_FLAG_COUNT > 8)
		f1 = OS_TASK_REG_1;
		OS_TASK_REG_1 = 0;
#endif // OS_TASK_FLAG_COUNT > 8
#if (OS_TASK_FLAG_COUNT > 16)
		f2 = OS_TASK_REG_2;
		OS_TASK_REG_2 = 0;
#endif // OS_TASK_FLAG_COUNT > 16
#if (OS_TASK_FLAG_COUNT > 24)
		f3 = OS_TASK_REG_3;
		OS_TASK_REG_3 = 0;
#endif // OS_TASK_FLAG_COUNT > 24
	}
#if (OS_TASK_FLAG_COUNT > 8)
	if(f0 != 0)
#endif // OS_TASK_FLAG_COUNT > 8
		task_sched_flags(0, f0);
#if (OS_TASK_FLAG_COUNT > 8)
	if(f1 != 0)
		task_sched_flags(8, f1);
#endif // OS_TASK_FLAG_COUNT > 8
#if (OS_TASK_FLAG_COUNT > 16)
	if(f2 != 0)
		task_sched_flags(16, f2);
#endif // OS_TASK_FLAG_COUNT > 16
#if (OS_TASK_FLAG_COUNT > 24)
	if(f3 != 0)
		task_sched_flags(24, f3);
#endif // OS_TASK_FLAG_COUNT > 24
}

// -------------------------------------------------------------------------------------------------

void os_init()
{
#ifdef OS_WD_TIMEOUT
	wdt_enable(OS_WD_TIMEOUT);
#endif // OS_WD_TIMEOUT
#ifdef OS_STK_SIZE
	stk_init();
#endif // OS_STK_SIZE
	task_data_init();
	task_flag_bind(OS_TICK_UPD_FLAG_ID, &tick_upd_task, TASK_PRIORITY_NORMAL);
	sleep_enable();
}

void __builtin_unreachable(void);

__attribute__((noreturn))
void os_run()
{
	// Reset stack since we don't want to return.
	ATOMIC_BLOCK(ATOMIC_FORCEON) { SP = RAMEND; }

	// OS main loop.
	for(;;) {

		// -------------------------------------------------
		// Run tasks until task queues are empty.

		for(;;) {
			// Schedule the flag-triggered tasks.
			if(task_flag_check()) {
#ifdef OS_WD_TIMEOUT
				wdt_reset();
#endif // OS_WD_TIMEOUT
				task_flag_sched_all();
			}
			// Run the next task.
#ifdef OS_WD_TIMEOUT
			wdt_reset();
#endif // OS_WD_TIMEOUT
			if(!task_run_next())
				break;
#ifdef OS_STK_SIZE
			stk_check_cookie();
#endif // OS_STK_SIZE
		}

		// -------------------------------------------------
		// Wait for the interrupt.

		// Disable interrupt and ensure no task flags are set.
		cli();
		if(!task_flag_check()) {
			// Wait for an interrupt.
#ifdef OS_WD_TIMEOUT
			wdt_reset();
#endif // OS_WD_TIMEOUT
#ifdef OS_BOD_DISABLE
			if(SMCR & (1<<SM1)) {
				MCUCR = (1<<BODS)|(1<<BODSE);
				MCUCR = 1<<BODS;
			}
#endif // OS_BOD_DISABLE
			sei();
			sleep_cpu();
		} else {
			// Have task flag(s) set, return to the task loop.
			sei();
		}
	}

	// prevent noreturn warning
	__builtin_unreachable();
}

// -------------------------------------------------------------------------------------------------
