// -------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "lib/os/os.h"
#include "lib/disp.h"
#include "ro.h"
#include "menu.h"
#include "config.h"
#include "hwconf.h"

// -------------------------------------------------------------------------------------------------

extern uint16_t ain_c;
extern uint16_t vin;

// -------------------------------------------------------------------------------------------------
// Beep

static uint8_t beep_tmr;

void beep(uint8_t dur)
{
	BUZZ_ON();
	beep_tmr = dur;
}

// -------------------------------------------------------------------------------------------------
// Display

static const uint8_t msg_blank[3] PROGMEM = { SSEG_EMPTY, SSEG_EMPTY, SSEG_EMPTY };
static const uint8_t msg_off[3]   PROGMEM = { SSEG_0,     SSEG_F,     SSEG_F     };
static const uint8_t msg_on[3]    PROGMEM = { SSEG_0,     SSEG_n,     SSEG_EMPTY };
static const uint8_t msg_flu[3]   PROGMEM = { SSEG_F,     SSEG_L,     SSEG_U     };
static const uint8_t msg_dry[3]   PROGMEM = { SSEG_d,     SSEG_r,     SSEG_Y     };
static const uint8_t msg_err[3]   PROGMEM = { SSEG_E,     SSEG_r,     SSEG_r     };
static const uint8_t msg_C00[3]   PROGMEM = { SSEG_C,     SSEG_0,     SSEG_0     };
static const uint8_t msg_C01[3]   PROGMEM = { SSEG_C,     SSEG_0,     SSEG_1     };
static const uint8_t msg_C02[3]   PROGMEM = { SSEG_C,     SSEG_0,     SSEG_2     };
static const uint8_t msg_C03[3]   PROGMEM = { SSEG_C,     SSEG_0,     SSEG_3     };
static const uint8_t msg_C04[3]   PROGMEM = { SSEG_C,     SSEG_0,     SSEG_4     };
static const uint8_t msg_C05[3]   PROGMEM = { SSEG_C,     SSEG_0,     SSEG_5     };
static const uint8_t msg_P00[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_0     };
static const uint8_t msg_P01[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_1     };
static const uint8_t msg_P02[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_2     };
static const uint8_t msg_P03[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_3     };
static const uint8_t msg_P04[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_4     };
static const uint8_t msg_P05[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_5     };
static const uint8_t msg_P06[3]   PROGMEM = { SSEG_P,     SSEG_0,     SSEG_6     };
static const uint8_t msg_clr[3]   PROGMEM = { SSEG_C,     SSEG_L,     SSEG_r     };
static const uint8_t msg_yes[3]   PROGMEM = { SSEG_Y,     SSEG_E,     SSEG_5     };
static const uint8_t msg_no[3]    PROGMEM = { SSEG_n,     SSEG_o,     SSEG_EMPTY };

static void disp_msg(const uint8_t *ptr)
{
	disp_buf[0] = pgm_read_byte(ptr + 0);
	disp_buf[1] = pgm_read_byte(ptr + 1);
	disp_buf[2] = pgm_read_byte(ptr + 2);
}

static void disp_uint(uint16_t val, uint8_t dp)
{
	uint8_t v0, v1, v2;
	if(val > 999) val = 999;
	v0 = (uint8_t)(val / 100);
	v1 = (uint8_t)((val / 10) % 10);
	v2 = (uint8_t)(val % 10);
	v0 = ((v0 != 0) || (dp == 1)) ?
		pgm_read_byte(sseg_digit + v0) : SSEG_EMPTY;
	v1 = ((v1 != 0) || (dp == 2) || (v0 != SSEG_EMPTY)) ?
		pgm_read_byte(sseg_digit + v1) : SSEG_EMPTY;
	v2 = pgm_read_byte(sseg_digit + v2);
	if(dp == 1) v0 |= SSEG_DP;
	else if(dp == 2) v1 |= SSEG_DP;
	else if(dp == 3) v2 |= SSEG_DP;
	disp_buf[0] = v0;
	disp_buf[1] = v1;
	disp_buf[2] = v2;
}

static void disp_uint32(uint32_t val)
{
	if(val <= 999) disp_uint((uint16_t)val, 0);
	else if(val <= 9999) disp_uint((uint16_t)(val / 10), 1);
	else if(val <= 99999) disp_uint((uint16_t)(val / 100), 2);
	else disp_uint((uint16_t)(val / 1000), 3);
}

static void disp_spin()
{
	static uint8_t n;
	disp_buf[0] = 0;
	disp_buf[1] = 0;
	disp_buf[2] = 0;
	switch(n) {
	case 0: disp_buf[0] = 0x01; break;
	case 1: disp_buf[1] = 0x01; break;
	case 2: disp_buf[2] = 0x01; break;
	case 3: disp_buf[2] = 0x02; break;
	case 4: disp_buf[2] = 0x04; break;
	case 5: disp_buf[2] = 0x08; break;
	case 6: disp_buf[1] = 0x08; break;
	case 7: disp_buf[0] = 0x08; break;
	case 8: disp_buf[0] = 0x10; break;
	case 9: disp_buf[0] = 0x20; break;
	}
	n = (n < 9) ? (n + 1) : 0;
}

// -------------------------------------------------------------------------------------------------
// Menu update

enum {
	BTN_STATE_RELEASED,
	BTN_STATE_PUSHED,
	BTN_STATE_LONGPUSHED,
};

enum {
	BTN_EV_NONE,
	BTN_EV_PUSH,
	BTN_EV_LONGPUSH,
};

static uint8_t btn_st = BTN_STATE_RELEASED;
static uint8_t btn_tmr = 0;

enum {
	MENU_STATE_DISPLAY,
	MENU_STATE_PARAM_SELECT,
	MENU_STATE_PARAM_EDIT,
	MENU_STATE_CONFIRM,
	MENU_STATE_COUNTER
};

enum {
	MENU_PARAM_OFF,					// OFF, On
	MENU_PARAM_FLUSH,				// FL
	MENU_PARAM_FILTER_TOTAL_TIME,	// C00
	MENU_PARAM_FILTER_WORK_TIME,	// C01
	MENU_PARAM_TOTAL_ON_TIME,		// C02
	MENU_PARAM_TOTAL_RUN_TIME,		// C03
	MENU_PARAM_NUM_STARTS,			// C04
	MENU_PARAM_NUM_FLUSHES,			// C05
	MENU_PARAM_MAN_FLUSH_TIME,		// P00 0..60/1 min
	MENU_PARAM_AUTO_FLUSH_TIME,		// P01 0..900/10 sec
	MENU_PARAM_FLUSH_WORK_THRES,	// P02 0..990/10 min
	MENU_PARAM_FLUSH_TOTAL_THRES,	// P03 0..240/5 hr
	MENU_PARAM_TIMEOUT_THRES,		// P04 0..360/5 min
	MENU_PARAM_NOWATER_THRES,		// P05 0..60/1 sec
	MENU_PARAM_EXTRA_TIME,			// P06 0..360/5 sec
	MENU_PARAM_FILTER_RESET,		// CLr
	MENU_PARAM_AIN_C,				// X.XX
	MENU_PARAM_VIN,					// XX.X
	MENU_PARAM_COUNT
};

static uint8_t lamp_state;
static uint8_t menu_state;
static uint8_t menu_cur_item;
static uint8_t menu_confirm;
static uint16_t menu_val_min;
static uint16_t menu_val_max;
static uint16_t menu_val_cur;
static uint16_t menu_val_step;
static uint16_t menu_hide_tmr;

// 48ms timer
static void menu_poll(struct tmr_interval *tmr)
{
	uint8_t btn_ev = BTN_EV_NONE;

	// -----------------------------------------------------
	// Watchdog reset
	wdt_reset();

	// -----------------------------------------------------
	// Beep update
	if(BUZZ_IS_ON() && (--beep_tmr == 0))
		BUZZ_OFF();

	// -----------------------------------------------------
	// Button poll
	if(BTN_IS_PSH()) {
		if(btn_st == BTN_STATE_RELEASED) {
			btn_st = BTN_STATE_PUSHED;
			beep(1);
		}
		if((btn_st == BTN_STATE_PUSHED) && (btn_tmr++ == BTN_LONGPUSH_THRES / 48)) {
			btn_st = BTN_STATE_LONGPUSHED;
			btn_ev = BTN_EV_LONGPUSH;
			beep(5);
		}
	} else if(btn_st != BTN_STATE_RELEASED) {
		if(btn_st == BTN_STATE_PUSHED)
			btn_ev = BTN_EV_PUSH;
		btn_st = BTN_STATE_RELEASED;
		btn_tmr = 0;
	}

	// -----------------------------------------------------
	// Menu -> Auto Hide
	if(btn_ev != BTN_EV_NONE)
		menu_hide_tmr = t_sec;
	if((menu_state != MENU_STATE_DISPLAY) && (t_sec - menu_hide_tmr > 60)) {
		menu_state = MENU_STATE_DISPLAY;
		beep(1);
	}

	// -----------------------------------------------------
	// Menu -> Display
	if(menu_state == MENU_STATE_DISPLAY)
	{
		uint8_t st = ro_get_state();
		switch(st) {
		case RO_DISABLED:
		case RO_OFF:
			disp_msg(msg_blank);
			break;
		case RO_IDLE:
			disp_uint((uint16_t)(ro_get_filter_total_time() / 86400), 0);
			break;
		case RO_WORK:
			disp_uint((uint16_t)(ro_get_current_work_time() / 6), 2);
			break;
		case RO_FLUSH:
			disp_spin();
			break;
		case RO_NOWATER:
			disp_msg(msg_dry);
			break;
		case RO_TIMEOUT:
			disp_msg(msg_err);
			break;
		}
		if(btn_ev == BTN_EV_PUSH) {
			if((st == RO_FLUSH) || (st == RO_TIMEOUT)) {
				ro_reset();
			} else {
				lamp_state = !lamp_state;
				ro_set_lamp(lamp_state);
			}
		} else if(btn_ev == BTN_EV_LONGPUSH) {
			menu_state = MENU_STATE_PARAM_SELECT;
			menu_cur_item = 0;
		}
		btn_ev = 0;
	}

	// -----------------------------------------------------
	// Menu -> Param select
	if(menu_state == MENU_STATE_PARAM_SELECT)
	{
		if(btn_ev == BTN_EV_PUSH) {
			menu_cur_item = (menu_cur_item < MENU_PARAM_COUNT - 1) ? (menu_cur_item + 1) : 0;
		} else if(btn_ev == BTN_EV_LONGPUSH) {
			switch(menu_cur_item) {
			case MENU_PARAM_OFF:
				if(ro_get_state() != RO_OFF) {
					ro_save_ee();
					ro_set_off(1);
				} else {
					ro_set_off(0);
				}
				menu_state = MENU_STATE_DISPLAY;
				break;
			case MENU_PARAM_FLUSH:
				ro_start_flush();
				menu_state = MENU_STATE_DISPLAY;
				break;
			case MENU_PARAM_FILTER_TOTAL_TIME:
			case MENU_PARAM_FILTER_WORK_TIME:
			case MENU_PARAM_TOTAL_ON_TIME:
			case MENU_PARAM_TOTAL_RUN_TIME:
			case MENU_PARAM_NUM_STARTS:
			case MENU_PARAM_NUM_FLUSHES:
				menu_state = MENU_STATE_COUNTER;
				break;
			case MENU_PARAM_MAN_FLUSH_TIME:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t)(ro_get_man_flush_time() / 60);
				menu_val_min = 0;
				menu_val_max = 60;
				menu_val_step = 1;
				break;
			case MENU_PARAM_AUTO_FLUSH_TIME:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t) ro_get_auto_flush_time();
				menu_val_min = 0;
				menu_val_max = 900;
				menu_val_step = 10;
				break;
			case MENU_PARAM_FLUSH_WORK_THRES:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t) (ro_get_flush_work_thres() / 60);
				menu_val_min = 0;
				menu_val_max = 990;
				menu_val_step = 10;
				break;
			case MENU_PARAM_FLUSH_TOTAL_THRES:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t) (ro_get_flush_total_thres() / 3600);
				menu_val_min = 0;
				menu_val_max = 240;
				menu_val_step = 5;
				break;
			case MENU_PARAM_TIMEOUT_THRES:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t) (ro_get_timeout_thres() / 60);
				menu_val_min = 0;
				menu_val_max = 360;
				menu_val_step = 5;
				break;
			case MENU_PARAM_NOWATER_THRES:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t) ro_get_nowater_thres();
				menu_val_min = 0;
				menu_val_max = 60;
				menu_val_step = 1;
				break;
			case MENU_PARAM_EXTRA_TIME:
				menu_state = MENU_STATE_PARAM_EDIT;
				menu_val_cur = (uint16_t) ro_get_extra_time();
				menu_val_min = 0;
				menu_val_max = 360;
				menu_val_step = 5;
				break;
			case MENU_PARAM_FILTER_RESET:
				menu_confirm = 0;
				menu_state = MENU_STATE_CONFIRM;
				break;
			case MENU_PARAM_AIN_C:
			case MENU_PARAM_VIN:
				menu_state = MENU_STATE_DISPLAY;
				break;
			}
		}
		switch(menu_cur_item) {
		case MENU_PARAM_OFF: disp_msg((ro_get_state() != RO_OFF) ? msg_off : msg_on); break;
		case MENU_PARAM_FLUSH: disp_msg(msg_flu); break;
		case MENU_PARAM_FILTER_TOTAL_TIME: disp_msg(msg_C00); break;
		case MENU_PARAM_FILTER_WORK_TIME: disp_msg(msg_C01); break;
		case MENU_PARAM_TOTAL_ON_TIME: disp_msg(msg_C02); break;
		case MENU_PARAM_TOTAL_RUN_TIME: disp_msg(msg_C03); break;
		case MENU_PARAM_NUM_STARTS: disp_msg(msg_C04); break;
		case MENU_PARAM_NUM_FLUSHES: disp_msg(msg_C05); break;
		case MENU_PARAM_MAN_FLUSH_TIME: disp_msg(msg_P00); break;
		case MENU_PARAM_AUTO_FLUSH_TIME: disp_msg(msg_P01); break;
		case MENU_PARAM_FLUSH_WORK_THRES: disp_msg(msg_P02); break;
		case MENU_PARAM_FLUSH_TOTAL_THRES: disp_msg(msg_P03); break;
		case MENU_PARAM_TIMEOUT_THRES: disp_msg(msg_P04); break;
		case MENU_PARAM_NOWATER_THRES: disp_msg(msg_P05); break;
		case MENU_PARAM_EXTRA_TIME: disp_msg(msg_P06); break;
		case MENU_PARAM_FILTER_RESET: disp_msg(msg_clr); break;
		case MENU_PARAM_AIN_C: disp_uint(ain_c / 10, 1); break;
		case MENU_PARAM_VIN: disp_uint(vin / 100, 2); break;
		}
		btn_ev = 0;
	}


	// -----------------------------------------------------
	// Menu -> Param edit
	if(menu_state == MENU_STATE_PARAM_EDIT)
	{
		if(btn_ev == BTN_EV_PUSH) {
			menu_val_cur += menu_val_step;
			if(menu_val_cur > menu_val_max)
				menu_val_cur = menu_val_min;
		} else if(btn_ev == BTN_EV_LONGPUSH) {
			switch(menu_cur_item) {
			case MENU_PARAM_MAN_FLUSH_TIME:
				ro_set_man_flush_time(menu_val_cur * 60ul);
				break;
			case MENU_PARAM_AUTO_FLUSH_TIME:
				ro_set_auto_flush_time(menu_val_cur);
				break;
			case MENU_PARAM_FLUSH_WORK_THRES:
				ro_set_flush_work_thres(menu_val_cur * 60ul);
				break;
			case MENU_PARAM_FLUSH_TOTAL_THRES:
				ro_set_flush_total_thres(menu_val_cur * 3600ul);
				break;
			case MENU_PARAM_TIMEOUT_THRES:
				ro_set_timeout_thres(menu_val_cur * 60ul);
				break;
			case MENU_PARAM_NOWATER_THRES:
				ro_set_nowater_thres(menu_val_cur);
				break;
			case MENU_PARAM_EXTRA_TIME:
				ro_set_extra_time(menu_val_cur);
				break;
			}
			menu_state = MENU_STATE_PARAM_SELECT;
		}
		disp_uint(menu_val_cur, 0);
		btn_ev = 0;
	}

	// -----------------------------------------------------
	// Menu -> Counter
	if(menu_state == MENU_STATE_COUNTER) {
		if(btn_ev == BTN_EV_PUSH) {
			menu_state = MENU_STATE_PARAM_SELECT;
		} else if(btn_ev == BTN_EV_LONGPUSH) {
			menu_state = MENU_STATE_DISPLAY;
		}
		switch(menu_cur_item) {
		case MENU_PARAM_FILTER_TOTAL_TIME:
			disp_uint32(ro_get_filter_total_time() / 3600);
			break;
		case MENU_PARAM_FILTER_WORK_TIME:
			disp_uint32(ro_get_filter_work_time() / 3600);
			break;
		case MENU_PARAM_TOTAL_ON_TIME:
			disp_uint32(ro_get_total_on_time() / 3600);
			break;
		case MENU_PARAM_TOTAL_RUN_TIME:
			disp_uint32(ro_get_total_run_time() / 3600);
			break;
		case MENU_PARAM_NUM_STARTS:
			disp_uint32(ro_get_num_starts());
			break;
		case MENU_PARAM_NUM_FLUSHES:
			disp_uint32(ro_get_num_flushes());
			break;
		default:
			disp_msg(msg_blank);
			break;
		}
		btn_ev = 0;
	}

	// -----------------------------------------------------
	// Menu -> Confirmation
	if(menu_state == MENU_STATE_CONFIRM) {
		if(btn_ev == BTN_EV_PUSH) {
			menu_confirm = !menu_confirm;
		} else if(btn_ev == BTN_EV_LONGPUSH) {
			if(menu_confirm) {
				switch(menu_cur_item) {
				case MENU_PARAM_FILTER_RESET:
					ro_filter_reset();
					break;
				}
				menu_state = MENU_STATE_DISPLAY;
			} else {
				menu_state = MENU_STATE_PARAM_SELECT;
			}
		}
		disp_msg(menu_confirm ? msg_yes : msg_no);
		btn_ev = 0;
	}
}

// -------------------------------------------------------------------------------------------------
// Menu init

static struct tmr_interval menu_tmr = TMR_INTERVAL(menu_poll, T_MS(48));

void menu_enable()
{
	wdt_enable(WDTO_250MS);
	BTN_ENABLE();
	BUZZ_ENABLE();
	tmr_interval_set(&menu_tmr, TMR_UNIT_TICK, 0);
	menu_state = MENU_STATE_DISPLAY;
	lamp_state = 0;
}

void menu_disable()
{
	tmr_interval_cancel(&menu_tmr);
	btn_st = BTN_STATE_RELEASED;
	btn_tmr = 0;

	BUZZ_DISABLE();
	beep_tmr = 0;

	BTN_DISABLE();

	disp_buf[0] = 0;
	disp_buf[1] = 0;
	disp_buf[2] = 0;

	wdt_disable();
}

// -------------------------------------------------------------------------------------------------
