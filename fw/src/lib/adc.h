// -------------------------------------------------------------------------------------------------

#pragma once

#ifndef __ASSEMBLER__
#include <stdint.h>
#include "../hwconf.h"
#endif // __ASSEMBLER__

// -------------------------------------------------------------------------------------------------

#define ADC_NSAMP		8

#ifndef __ASSEMBLER__

// Continuous read channel list
enum {
	ADC_AIN_C,
	ADC_VDC,
	ADC_NCH
};
#define ADC_CH_MUX		{ ADMUX_AIN_C, ADMUX_VDC }

// ADC continuous read
// NSAMP samples gathered, first sample and 2 extreme values are discarded
// (NSAMP-3) remaining samples are summed, so full range is 0..(1024*(NSAMP-3))
typedef void (*adc_callback_t)(const uint16_t *res);
void adc_read_enable(adc_callback_t callback);
void adc_read_disable();

// read single sample when continuous read disabled
// (no filtering, full range is 0..1024)
uint16_t adc_read_single(uint8_t mux);

#endif // __ASSEMBLER__

// -------------------------------------------------------------------------------------------------
