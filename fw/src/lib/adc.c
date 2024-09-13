// -------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include <util/delay.h>
#include "os/os.h"
#include "adc.h"

// -------------------------------------------------------------------------------------------------

extern uint16_t adc_buf[ADC_NSAMP];
void adc_start(uint8_t mux);

static uint16_t adc_read_res()
{
	uint8_t i;
	uint16_t res = 0, a = 0, b = 0xffff;
	for(i = 1; i < ADC_NSAMP; i++) {
		uint16_t v = adc_buf[i];
		res += v;
		if(v > a) a = v;
		if(v < b) b = v;
	}
	return res - (a + b);
}

// -------------------------------------------------------------------------------------------------

static const uint8_t adc_ch_mux[ADC_NCH] = ADC_CH_MUX;
static uint16_t adc_res[ADC_NCH];
static uint8_t adc_ch_cur;
static adc_callback_t adc_callback;

static void adc_buf_done(struct task_handle *task)
{
	adc_res[adc_ch_cur++] = adc_read_res();
	if(adc_ch_cur == ADC_NCH) {
		adc_ch_cur = 0;
		adc_callback(adc_res);
		// adc refresh can be disabled by the callback
		if(!(ADCSRA & (1<<ADEN)))
			return;
	}
	adc_start(adc_ch_mux[adc_ch_cur]);
}

// -------------------------------------------------------------------------------------------------

void adc_read_enable(adc_callback_t callback)
{
	static struct task_handle adc_buf_done_task = TASK_HANDLE(adc_buf_done);
	task_flag_bind(ADC_BUF_DONE_FLAG_ID, &adc_buf_done_task, TASK_PRIORITY_NORMAL);
	adc_callback = callback;

	ADMUX = adc_ch_mux[0];
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1); // F_CPU/32

	adc_ch_cur = 0;
	adc_start(adc_ch_mux[0]);
}

void adc_read_disable()
{
	ADCSRA = 0;
	ADMUX = 0;
}

// -------------------------------------------------------------------------------------------------

uint16_t adc_read_single(uint8_t mux)
{
	uint16_t data;
	ADMUX = mux;
	ADCSRA = (1<<ADEN)|(1<<ADPS2); // F_CPU/16
	ADCSRA |= 1<<ADSC; loop_until_bit_is_set(ADCSRA, ADIF);
	ADCSRA |= 1<<ADSC; loop_until_bit_is_set(ADCSRA, ADIF);
	data = ADC;
	ADCSRA = ADCSRA;
	ADCSRA = 0;
	ADMUX = 0;
	return data;
}

// -------------------------------------------------------------------------------------------------
