//--------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "lib/os/os.h"
#include "lib/adc.h"
#include "lib/rtc.h"
#include "ro.h"
#include "menu.h"
#include "config.h"
#include "hwconf.h"

//--------------------------------------------------------------------------------------------------
// PWR

static void adc_callback(const uint16_t *res);
static uint8_t is_pwr_on;

static void pwr_on()
{
	TICK_DISP_ENABLE();					// enable display and system tick
	adc_read_enable(adc_callback);		// enable ADC refresh
	menu_enable();						// enable ui
	ro_enable();						// enable RO controller

	// power on mode
	set_sleep_mode(SLEEP_MODE_IDLE);
	is_pwr_on = 1;
}

static void pwr_off()
{
	ro_disable();						// disable RO controller
	menu_disable();						// disable ui
	adc_read_disable();					// disable ADC refresh
	TICK_DISP_DISABLE();				// disable display and system tick

	// power save mode
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	is_pwr_on = 0;
}

//--------------------------------------------------------------------------------------------------
// ADC callback. 5-sample average: 0..(1024*5)

uint16_t ain_c;
uint16_t vin;

static void adc_callback(const uint16_t *res)
{
	ain_c = (uint16_t) (res[ADC_AIN_C] * 1000UL >> 10);	// 1024*5 -> 5000mV FS
	vin = res[ADC_VDC] * 10 + VIN_DROP;					// 1024*5 -> 51200mV FS + VDC diode drop

	// pwr off mode by the input voltage threshold
	if(vin < VIN_THRES_PWRDOWN)
		pwr_off();
}

//--------------------------------------------------------------------------------------------------
// RTC tick (2s)

static void rtc_tick(struct task_handle *task)
{
	// wake up by the input voltage threshold
	// 1024->51.2V FS + VDC diode drop
	if(!is_pwr_on && (adc_read_single(ADMUX_VDC) >= (VIN_THRES_PWRUP-VIN_DROP)/50))
		pwr_on();
	// update rtc second counter
	t_rtc_sec += 2;
}

static void rtc_init()
{
	static struct task_handle rtc_tick_task = TASK_HANDLE(rtc_tick);
	RTC_TICK_INIT();
	task_flag_bind(RTC_TICK_FLAG_ID, &rtc_tick_task, TASK_PRIORITY_NORMAL);
}

//--------------------------------------------------------------------------------------------------

static void mcu_init()
{
	// initialize MCU
	MCUSR = 0;
	wdt_disable();
	clock_prescale_set(clock_div_2); // F_CPU=4MHz
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);

	// initialize GPIO
//	PORTB = PORTB_INIT;
	DDRB = DDRB_INIT;
//	PORTC = PORTC_INIT;
	DDRC = DDRC_INIT;
//	PORTD = PORTD_INIT;
	DDRD = DDRD_INIT;

	ACSR = 1<<ACD;
	PRR = (1<<PRTWI)|(1<<PRSPI)|(1<<PRUSART0)|(1<<PRTIM0)|(1<<PRTIM1);

	sei();
}

int main()
{
	mcu_init();
	os_init();
	rtc_init();
	ro_load_ee();
	os_run();
}

//--------------------------------------------------------------------------------------------------
