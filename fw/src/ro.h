// -------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

// -------------------------------------------------------------------------------------------------

enum {
	RO_DISABLED,
	RO_OFF,
	RO_IDLE,
	RO_WORK,
	RO_FLUSH,
	RO_NOWATER,
	RO_TIMEOUT
};

uint8_t ro_get_state();

// commands
void ro_enable();
void ro_disable();
void ro_reset();
void ro_start_flush();
void ro_filter_reset();
void ro_set_lamp(uint8_t on);
void ro_set_off(uint8_t off);

// config
void ro_set_nowater_thres(uint32_t thres);
uint32_t ro_get_nowater_thres();
void ro_set_timeout_thres(uint32_t thres);
uint32_t ro_get_timeout_thres();
void ro_set_flush_work_thres(uint32_t thres);
uint32_t ro_get_flush_work_thres();
void ro_set_flush_total_thres(uint32_t thres);
uint32_t ro_get_flush_total_thres();
void ro_set_auto_flush_time(uint32_t val);
uint32_t ro_get_auto_flush_time();
void ro_set_man_flush_time(uint32_t val);
uint32_t ro_get_man_flush_time();
void ro_set_extra_time(uint32_t val);
uint32_t ro_get_extra_time();

// data
uint32_t ro_get_num_starts();
uint32_t ro_get_num_flushes();
uint32_t ro_get_filter_total_time();
uint32_t ro_get_filter_work_time();
uint32_t ro_get_current_work_time();
uint32_t ro_get_total_on_time();
uint32_t ro_get_total_run_time();

void ro_load_ee();
void ro_save_ee();

// -------------------------------------------------------------------------------------------------
