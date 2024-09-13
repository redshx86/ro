// -------------------------------------------------------------------------------------------------

#pragma once

// -------------------------------------------------------------------------------------------------

#define BTN_LONGPUSH_THRES				576		// 48ms*12

#define VIN_DROP						3000	// Diode drop 0.75V*4
#define VIN_THRES_PWRDOWN				15000	// Pwr down when below 15V
#define VIN_THRES_PWRUP					21000	// Pwr up when above 21V

#define RO_CFG_MAN_FLUSH_TIME			900		// P00 0..60/1 min
#define RO_CFG_AUTO_FLUSH_TIME			60		// P01 0..900/10 sec
#define RO_CFG_FLUSH_WORK_THRES			7200	// P02 0..990/10 min
#define RO_CFG_FLUSH_TOTAL_THRES		86400	// P03 0..240/5 hr
#define RO_CFG_TIMEOUT_THRES			900		// P04 0..360/5 min
#define RO_CFG_NOWATER_THRES			5		// P05 0..60/1 sec
#define RO_CFG_EXTRA_TIME				30		// P06 0..360/5 sec

// -------------------------------------------------------------------------------------------------
