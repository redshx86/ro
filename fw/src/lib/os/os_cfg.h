// -------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "../../hwconf.h"

// -------------------------------------------------------------------------------------------------
// OS options.

// Enable call parameter checking. Comment to disable.
// Passing illegal parameters to the OS calls will cause undefined results, if disabled.
#define OS_PARAM_CHECK

// -------------------------------------------------------------------------------------------------
// Watchdog configuration.

// Watchdog timeout:
// - Max time of the scheduler cycle.
// - Max time executing a task.
// - Max time waiting for an interrupt.
// Comment to disable wathdog management by the OS.
//#define OS_WD_TIMEOUT				WDTO_15MS

// -------------------------------------------------------------------------------------------------
// BOD configuration.

// Turn off BOD when entering power-save or power-down (SM1=1).
// Comment to disable.
#define OS_BOD_DISABLE

// -------------------------------------------------------------------------------------------------
// Stack checking configuration.

// Statck size limit. Comment to disable stack checking by the OS.
#define OS_STK_SIZE					256

// Stack overflow action.
//#define STK_OVF_ACTION()			LED_PORT |= LED_ALL

// Stack function configuration. Comment to disable.
#define OS_STK_MAX_USAGE	// implement the stk_max_usage function.

// Set heap end before the stack. Comment to disable.
#define OS_STK_ADJUST_HEAP

// -------------------------------------------------------------------------------------------------
// Tick counters configuration.

// System tick frequency as floating-point constant. Must be same as OS_TICK_SEC_DIV.
// Comment to disable T_US and T_MS macros.
//#define OS_TICK_FREQ				(F_CPU / 16384.0)
#define OS_TICK_FREQ				(F_CPU / 19200.0) // 4.8ms @ 4MHz

// System tick frequency as natural fraction. Must be same as OS_TICK_FREQ.
// Comment to disable the second counter.
// Comment fractional part if zero.
// Here are examples for the F_CPU/16384 tick rate.
//#if F_CPU == 8000000
//  #define OS_TICK_SEC_DIV_INT		488		// int part
//  #define OS_TICK_SEC_DIV_P		9		// frac part numerator
//  #define OS_TICK_SEC_DIV_Q		32		// frac part denominator
//#elif F_CPU == 16000000
//  #define OS_TICK_SEC_DIV_INT		976		// int part
//  #define OS_TICK_SEC_DIV_P		9		// frac part numerator
//  #define OS_TICK_SEC_DIV_Q		16		// frac part denominator
//#elif F_CPU == 20000000
//  #define OS_TICK_SEC_DIV_INT		1220	// int part
//  #define OS_TICK_SEC_DIV_P		45		// frac part numerator
//  #define OS_TICK_SEC_DIV_Q		64		// frac part denominator
//#elif F_CPU == 16384000
//  #define OS_TICK_SEC_DIV_INT		1000	// int part
////#define OS_TICK_SEC_DIV_P		1		// frac part numerator
////#define OS_TICK_SEC_DIV_Q		1		// frac part denominator
//#endif // F_CPU
#define OS_TICK_SEC_DIV_INT			208		// int part
#define OS_TICK_SEC_DIV_P			1		// frac part numerator
#define OS_TICK_SEC_DIV_Q			3		// frac part denominator

// Tick function configuration. Comment to disable.
#define OS_TICK_READ		// implement the t_tick_read function
#define OS_TICK_WAIT		// implement the t_tick_wait function

// -------------------------------------------------------------------------------------------------
// Task configuration.

// Number of task flags. Mandatory. Must be 1..32.
#define OS_TASK_FLAG_COUNT			12

// Task flag registers. Comment to use a global variable instead.
#ifdef GPIOR0
  #define OS_TASK_REG_0				GPIOR0
  #define OS_TASK_REG_1				GPIOR1
  #define OS_TASK_REG_2				GPIOR2
#endif // GPIOR0

// Max number of dynamic tasks. Comment to disable dynamic task allocation.
#define OS_TASK_DYN_COUNT			8

// -------------------------------------------------------------------------------------------------
// Timer

// Timer feature configuration. Comment to disable.
#define OS_TMR				// Timer module
#define OS_TMR_TICK			// Tick time unit
#define OS_TMR_SECOND		// Second time unit
#define OS_TMR_ONESHOT		// One-shot timer
#define OS_TMR_INTERVAL		// Interval timer

// -------------------------------------------------------------------------------------------------
// Task Flags

// OS task updating the tick counters. Mandatory.
// used by asm. do not change
#define OS_TICK_UPD_FLAG_ID			0
#define OS_TICK_UPD_FLAG			TASK_FLAG_0

// User flags
// used by asm. do not change
#define RTC_TICK_FLAG_ID			1
#define RTC_TICK_FLAG				TASK_FLAG_1

// used by asm. do not change
#define ADC_BUF_DONE_FLAG_ID		2
#define ADC_BUF_DONE_FLAG			TASK_FLAG_2

// -------------------------------------------------------------------------------------------------

void os_init();
void os_run();

// -------------------------------------------------------------------------------------------------
