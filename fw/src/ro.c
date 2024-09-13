// -------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/eeprom.h>
#include "lib/os/os.h"
#include "lib/rtc.h"
#include "menu.h"
#include "ro.h"
#include "config.h"
#include "hwconf.h"

// -------------------------------------------------------------------------------------------------
// I/O

#define INLET_SW_ON()			DI_A_IS_ON()
#define REFILL_SW_ON()			DI_B_IS_ON()
#define WORK_ON()				DQ_A_ON()
#define WORK_OFF()				DQ_A_OFF()
#define BYPASS_ON()				DQ_B_ON()
#define BYPASS_OFF()			DQ_B_OFF()
#define LAMP_ON()				DQ_C_ON()
#define LAMP_OFF()				DQ_C_OFF()

extern uint16_t ain_c;

// -------------------------------------------------------------------------------------------------
// settings

static uint8_t EEMEM ee_ro_cfg_off;
static uint32_t EEMEM ee_ro_cfg_nowater_thres;
static uint32_t EEMEM ee_ro_cfg_timeout_thres;
static uint32_t EEMEM ee_ro_cfg_flush_work_thres;
static uint32_t EEMEM ee_ro_cfg_flush_total_thres;
static uint32_t EEMEM ee_ro_cfg_auto_flush_time;
static uint32_t EEMEM ee_ro_cfg_man_flush_time;
static uint32_t EEMEM ee_ro_cfg_extra_time;

static uint8_t ro_cfg_off							= 0;
static uint32_t ro_cfg_nowater_thres				= RO_CFG_NOWATER_THRES;
static uint32_t ro_cfg_timeout_thres				= RO_CFG_TIMEOUT_THRES;
static uint32_t ro_cfg_flush_work_thres				= RO_CFG_FLUSH_WORK_THRES;
static uint32_t ro_cfg_flush_total_thres			= RO_CFG_FLUSH_TOTAL_THRES;
static uint32_t ro_cfg_auto_flush_time				= RO_CFG_AUTO_FLUSH_TIME;
static uint32_t ro_cfg_man_flush_time				= RO_CFG_MAN_FLUSH_TIME;
static uint32_t ro_cfg_extra_time					= RO_CFG_EXTRA_TIME;

// -------------------------------------------------------------------------------------------------
// data

static uint32_t EEMEM ee_ro_data_num_starts;
static uint32_t EEMEM ee_ro_data_num_flushes;
static uint32_t EEMEM ee_ro_data_filter_total_time;
static uint32_t EEMEM ee_ro_data_filter_work_time;
static uint32_t EEMEM ee_ro_data_filter_noflushwrk_time;
static uint32_t EEMEM ee_ro_data_total_on_time;
static uint32_t EEMEM ee_ro_data_total_run_time;

static uint32_t ro_data_num_starts;
static uint32_t ro_data_num_flushes;
static uint32_t ro_data_filter_total_time;
static uint32_t ro_data_filter_work_time;
static uint32_t ro_data_filter_noflushwrk_time;
static uint32_t ro_data_total_on_time;
static uint32_t ro_data_total_run_time;

// -------------------------------------------------------------------------------------------------
// 48ms timer

static uint8_t ro_state;
static uint32_t ro_flush_time;
static uint32_t ro_start_mark;
static uint32_t ro_nowater_mark;
static uint32_t ro_last_flush_mark;
static uint32_t ro_total_upd_mark;
static uint32_t ro_data_save_mark;
static uint32_t ro_work_sw_mark;

static void ro_update_total_time()
{
	ro_data_filter_total_time += t_rtc_sec - ro_total_upd_mark;
	ro_data_total_on_time += t_rtc_sec - ro_total_upd_mark;
	ro_total_upd_mark = t_rtc_sec;
}

static void ro_update_run_time()
{
	if((ro_state == RO_WORK) || (ro_state == RO_FLUSH)) {
		uint32_t elap = t_rtc_sec - ro_start_mark;
		ro_data_filter_work_time += elap;
		ro_data_total_run_time += elap;
		if(ro_state == RO_WORK)
			ro_data_filter_noflushwrk_time += elap;
		ro_start_mark = t_rtc_sec;
	}
}

static void ro_work()
{
	ro_update_run_time();
	WORK_ON();
	BYPASS_OFF();
	if((ro_state != RO_WORK) && (ro_state != RO_FLUSH))
		ro_data_num_starts++;
	ro_start_mark = t_rtc_sec;
	ro_work_sw_mark = t_rtc_sec;
	ro_state = RO_WORK;
}

static void ro_flush(uint32_t flush_time)
{
	ro_update_run_time();
	WORK_ON();
	BYPASS_ON();
	if((ro_state != RO_WORK) && (ro_state != RO_FLUSH))
		ro_data_num_starts++;
	if(ro_state != RO_FLUSH)
		ro_data_num_flushes++;
	ro_data_filter_noflushwrk_time = 0;
	ro_last_flush_mark = t_rtc_sec;
	ro_start_mark = t_rtc_sec;
	ro_flush_time = flush_time;
	ro_state = RO_FLUSH;
}

static void ro_idle()
{
	ro_update_run_time();
	WORK_OFF();
	BYPASS_OFF();
	ro_state = RO_IDLE;
}

uint8_t ro_get_state()
{
	return ro_state;
}

void ro_update(struct tmr_interval *tmr)
{
	static uint8_t beep_cnt;
	static uint8_t beep_tmr;

	uint32_t t = t_rtc_sec;

	switch(ro_state)
	{
	// -----------------------------------------------------
	// Idle
	case RO_IDLE:
		if(!INLET_SW_ON())
			break;
		// Idle -> Flush (total time since last flush)
		if( (ro_cfg_flush_total_thres != 0) &&
			(ro_cfg_auto_flush_time != 0) &&
			(t - ro_last_flush_mark >= ro_cfg_flush_total_thres) )
		{
			ro_flush(ro_cfg_auto_flush_time);
		}
		// Idle -> Work
		else if(REFILL_SW_ON()) {
			ro_work();
		}
		break;

	// -----------------------------------------------------
	// Work (refill)
	case RO_WORK:
		// Work -> Timeout
		if((ro_cfg_timeout_thres != 0) && (t - ro_start_mark > ro_cfg_timeout_thres)) {
			ro_idle();
			ro_state = RO_TIMEOUT;
		}
		// Work -> Idle/Flush
		else if(REFILL_SW_ON()) {
			ro_work_sw_mark = t;
		} else if(t - ro_work_sw_mark >= ro_cfg_extra_time) {
			ro_update_run_time();
			// Work -> Flush
			if( (ro_cfg_flush_work_thres != 0) &&
				(ro_cfg_auto_flush_time != 0) &&
				(ro_data_filter_noflushwrk_time >= ro_cfg_flush_work_thres) &&
				INLET_SW_ON() )
			{
				ro_flush(ro_cfg_auto_flush_time);
			}
			// Work -> Idle
			else {
				ro_idle();
				beep(5);
			}
		}
		break;

	// -----------------------------------------------------
	// Flush
	case RO_FLUSH:
		if(t - ro_start_mark < ro_flush_time)
			break;
		// Flush -> Work/Idle
		if(REFILL_SW_ON() && INLET_SW_ON()) {
			ro_work();
		} else {
			ro_idle();
			beep(5);
		}
		break;

	// -----------------------------------------------------
	// Nowater
	case RO_NOWATER:
		if(!INLET_SW_ON())
			break;
		beep(5);
		// Nowater -> Flush/Idle
		if(ro_cfg_auto_flush_time != 0) {
			ro_flush(ro_cfg_auto_flush_time);
		} else {
			ro_idle();
		}
		break;
	}

	// -----------------------------------------------------
	// Nowater check
	if(INLET_SW_ON()) {
		ro_nowater_mark = t;
	} else if( ((ro_state == RO_IDLE) || (ro_state == RO_WORK) || (ro_state == RO_FLUSH)) &&
	           (t - ro_nowater_mark >= ro_cfg_nowater_thres) )
	{
		ro_idle();
		ro_state = RO_NOWATER;
	}

	// -----------------------------------------------------
	// Alarm
	if((ro_state == RO_NOWATER) || (ro_state == RO_TIMEOUT)) {
		if((ro_state == RO_TIMEOUT) || (beep_cnt < 10)) {
			if(++beep_tmr == 14) {
				beep(7);
				beep_cnt++;
				beep_tmr = 0;
			}
		}
	} else {
		beep_tmr = 0;
		beep_cnt = 0;
	}

	// -----------------------------------------------------
	// Update data
	if(t != ro_total_upd_mark)
		ro_update_total_time();

	// -----------------------------------------------------
	// Save data
	if(t - ro_data_save_mark >= 86400)
		ro_save_ee();
}

// -------------------------------------------------------------------------------------------------
// commands

void ro_reset()
{
	if((ro_state != RO_FLUSH) && (ro_state != RO_TIMEOUT))
		return;
	ro_idle();
}

void ro_start_flush()
{
	if((ro_state != RO_IDLE) && (ro_state != RO_WORK) && (ro_state != RO_FLUSH))
		return;
	if((ro_cfg_man_flush_time == 0) || !INLET_SW_ON())
		return;
	ro_flush(ro_cfg_man_flush_time);
}

void ro_filter_reset()
{
	ro_update_total_time();
	ro_data_filter_total_time = 0;
	ro_data_filter_work_time = 0;
	ro_data_filter_noflushwrk_time = 0;
}

void ro_set_lamp(uint8_t on)
{
	if(ro_state == RO_DISABLED)
		return;
	if(on) {
		LAMP_ON();
	} else {
		LAMP_OFF();
	}
}

void ro_set_off(uint8_t off)
{
	if(off) {
		if(ro_cfg_off)
			return;
		ro_cfg_off = 1;
		eeprom_write_byte(&ee_ro_cfg_off, 1);
		ro_idle();
		ro_state = RO_OFF;
	} else {
		if(!ro_cfg_off)
			return;
		ro_cfg_off = 0;
		eeprom_write_byte(&ee_ro_cfg_off, 0);
		if(ro_state == RO_OFF)
			ro_state = RO_IDLE;
	}
}

// -------------------------------------------------------------------------------------------------
// config

void ro_set_nowater_thres(uint32_t thres)
{
	if(thres > 60)
		thres = RO_CFG_NOWATER_THRES;
	if(thres != ro_cfg_nowater_thres) {
		ro_cfg_nowater_thres = thres;
		eeprom_write_dword(&ee_ro_cfg_nowater_thres, thres);
	}
}

uint32_t ro_get_nowater_thres()
{
	return ro_cfg_nowater_thres;
}

void ro_set_timeout_thres(uint32_t thres)
{
	if(thres > 21600)
		thres = RO_CFG_TIMEOUT_THRES;
	if(thres != ro_cfg_timeout_thres) {
		ro_cfg_timeout_thres = thres;
		eeprom_write_dword(&ee_ro_cfg_timeout_thres, thres);
	}
}

uint32_t ro_get_timeout_thres()
{
	return ro_cfg_timeout_thres;
}

void ro_set_flush_work_thres(uint32_t thres)
{
	if(thres > 59400)
		thres = RO_CFG_FLUSH_WORK_THRES;
	if(thres != ro_cfg_flush_work_thres) {
		ro_cfg_flush_work_thres = thres;
		eeprom_write_dword(&ee_ro_cfg_flush_work_thres, thres);
	}
}

uint32_t ro_get_flush_work_thres()
{
	return ro_cfg_flush_work_thres;
}

void ro_set_flush_total_thres(uint32_t thres)
{
	if(thres > 864000)
		thres = RO_CFG_FLUSH_TOTAL_THRES;
	if(thres != ro_cfg_flush_total_thres) {
		ro_cfg_flush_total_thres = thres;
		eeprom_write_dword(&ee_ro_cfg_flush_total_thres, thres);
	}
}

uint32_t ro_get_flush_total_thres()
{
	return ro_cfg_flush_total_thres;
}

void ro_set_auto_flush_time(uint32_t val)
{
	if(val > 900)
		val = RO_CFG_AUTO_FLUSH_TIME;
	if(val != ro_cfg_auto_flush_time) {
		ro_cfg_auto_flush_time = val;
		eeprom_write_dword(&ee_ro_cfg_auto_flush_time, val);
	}
}

uint32_t ro_get_auto_flush_time()
{
	return ro_cfg_auto_flush_time;
}

void ro_set_man_flush_time(uint32_t val)
{
	if(val > 3600)
		val = RO_CFG_MAN_FLUSH_TIME;
	if(val != ro_cfg_man_flush_time) {
		ro_cfg_man_flush_time = val;
		eeprom_write_dword(&ee_ro_cfg_man_flush_time, val);
	}
}

uint32_t ro_get_man_flush_time()
{
	return ro_cfg_man_flush_time;
}

void ro_set_extra_time(uint32_t val)
{
	if(val > 360)
		val = RO_CFG_EXTRA_TIME;
	if(val != ro_cfg_extra_time) {
		ro_cfg_extra_time = val;
		eeprom_write_dword(&ee_ro_cfg_extra_time, val);
	}
}

uint32_t ro_get_extra_time()
{
	return ro_cfg_extra_time;
}

// -------------------------------------------------------------------------------------------------
// data

uint32_t ro_get_num_starts()
{
	return ro_data_num_starts;
}

uint32_t ro_get_num_flushes()
{
	return ro_data_num_flushes;
}

uint32_t ro_get_filter_total_time()
{
	return ro_data_filter_total_time +
		(t_rtc_sec - ro_total_upd_mark);
}

uint32_t ro_get_filter_work_time()
{
	uint32_t val = ro_data_filter_work_time;
	if((ro_state == RO_WORK) || (ro_state == RO_FLUSH))
		val += t_rtc_sec - ro_start_mark;
	return val;
}

uint32_t ro_get_current_work_time()
{
	if((ro_state != RO_WORK) && (ro_state != RO_FLUSH))
		return 0;
	return t_rtc_sec - ro_start_mark;
}

uint32_t ro_get_total_on_time()
{
	return ro_data_total_on_time +
		(t_rtc_sec - ro_total_upd_mark);
}

uint32_t ro_get_total_run_time()
{
	uint32_t val = ro_data_total_run_time;
	if((ro_state == RO_WORK) || (ro_state == RO_FLUSH))
		val += t_rtc_sec - ro_start_mark;
	return val;
}

// -------------------------------------------------------------------------------------------------

void ro_load_ee()
{
	ro_cfg_off = eeprom_read_byte(&ee_ro_cfg_off);
	if(ro_cfg_off > 1)	ro_cfg_off = 0;

	ro_set_nowater_thres(eeprom_read_dword(&ee_ro_cfg_nowater_thres));
	ro_set_timeout_thres(eeprom_read_dword(&ee_ro_cfg_timeout_thres));
	ro_set_flush_work_thres(eeprom_read_dword(&ee_ro_cfg_flush_work_thres));
	ro_set_flush_total_thres(eeprom_read_dword(&ee_ro_cfg_flush_total_thres));
	ro_set_auto_flush_time(eeprom_read_dword(&ee_ro_cfg_auto_flush_time));
	ro_set_man_flush_time(eeprom_read_dword(&ee_ro_cfg_man_flush_time));
	ro_set_extra_time(eeprom_read_dword(&ee_ro_cfg_extra_time));

	ro_data_num_starts = eeprom_read_dword(&ee_ro_data_num_starts);
	ro_data_num_flushes = eeprom_read_dword(&ee_ro_data_num_flushes);
	ro_data_filter_total_time = eeprom_read_dword(&ee_ro_data_filter_total_time);
	ro_data_filter_work_time = eeprom_read_dword(&ee_ro_data_filter_work_time);
	ro_data_filter_noflushwrk_time = eeprom_read_dword(&ee_ro_data_filter_noflushwrk_time);
	ro_data_total_on_time = eeprom_read_dword(&ee_ro_data_total_on_time);
	ro_data_total_run_time = eeprom_read_dword(&ee_ro_data_total_run_time);

	if(ro_data_num_starts == 0xffffffff)				ro_data_num_starts = 0;
	if(ro_data_num_flushes == 0xffffffff)				ro_data_num_flushes = 0;
	if(ro_data_filter_total_time == 0xffffffff)			ro_data_filter_total_time = 0;
	if(ro_data_filter_work_time == 0xffffffff)			ro_data_filter_work_time = 0;
	if(ro_data_filter_noflushwrk_time == 0xffffffff)	ro_data_filter_noflushwrk_time = 0;
	if(ro_data_total_on_time == 0xffffffff)				ro_data_total_on_time = 0;
	if(ro_data_total_run_time == 0xffffffff)			ro_data_total_run_time = 0;
}

void ro_save_ee()
{
	eeprom_update_dword(&ee_ro_data_num_starts, ro_data_num_starts);
	eeprom_update_dword(&ee_ro_data_num_flushes, ro_data_num_flushes);
	eeprom_update_dword(&ee_ro_data_filter_total_time, ro_data_filter_total_time);
	eeprom_update_dword(&ee_ro_data_filter_work_time, ro_data_filter_work_time);
	eeprom_update_dword(&ee_ro_data_filter_noflushwrk_time, ro_data_filter_noflushwrk_time);
	eeprom_update_dword(&ee_ro_data_total_on_time, ro_data_total_on_time);
	eeprom_update_dword(&ee_ro_data_total_run_time, ro_data_total_run_time);
	ro_data_save_mark = t_rtc_sec;
}

// -------------------------------------------------------------------------------------------------

static struct tmr_interval ro_update_tmr = TMR_INTERVAL(ro_update, T_MS(48));

void ro_enable()
{
	if(ro_state != RO_DISABLED)
		return;
	tmr_interval_set(&ro_update_tmr, TMR_UNIT_TICK, 0);
	ro_state = ro_cfg_off ? RO_OFF : RO_IDLE;
}

void ro_disable()
{
	if(ro_state == RO_DISABLED)
		return;
	ro_idle();
	LAMP_OFF();
	tmr_interval_cancel(&ro_update_tmr);
	ro_state = RO_DISABLED;
}

// -------------------------------------------------------------------------------------------------
