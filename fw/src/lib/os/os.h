// -------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "task.h"
#include "task_flg.h"
#include "tmr.h"
#include "os_cfg.h"

// -------------------------------------------------------------------------------------------------
// Stack checking.

// Assesses the max stack usage by checking how much stack bytes were modified since the start.
#if (defined OS_STK_SIZE) && (defined OS_STK_MAX_USAGE)
uint16_t stk_max_usage();
#endif // OS_STK_MAX_USAGE

// -------------------------------------------------------------------------------------------------
// Tick counters.

// Tick source counter updated by the user interrupt.
// Note: Set tick update flag after updating t_tick_src: TASK_FLAG_SET(OS_TICK_UPD_FLAG)
uint16_t volatile t_tick_src;

// Tick counter.
uint16_t t_tick;

// Defines a time interval as a number of ticks.
#ifdef OS_TICK_FREQ
#define T_US(n)		(uint16_t)(1e-6 * (n) * OS_TICK_FREQ + 0.5)
#define T_MS(n)		(uint16_t)(1e-3 * (n) * OS_TICK_FREQ + 0.5)
#endif // OS_TICK_FREQ

// Seconds counter.
#ifdef OS_TICK_SEC_DIV_INT
uint16_t t_sec;
#endif // OS_TICK_SEC_DIV_INT

// Reads t_tick_src atomically.
#ifdef OS_TICK_READ
uint16_t t_tick_read();
#endif // OS_TICK_READ

// Waits a number of ticks.
#ifdef OS_TICK_WAIT
void t_tick_wait(uint16_t n);
#endif // OS_TICK_WAIT

// -------------------------------------------------------------------------------------------------

void os_init();

__attribute__((noreturn))
void os_run();

// -------------------------------------------------------------------------------------------------
