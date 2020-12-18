/* SPDX-License-Identifier: GPL-2.0 */

enum ssd1306_cmd {
	//Fundamental commands:
	SET_CONTRAST_CTRL        = 0x81,
	ENTIRE_DISP_ON           = 0xA4,
	ENTIRE_DISP_OFF          = 0xA5,
	SET_DISP_NORMAL          = 0xA6,
	SET_DISP_INVERT          = 0xA7,
	SET_DISP_ON              = 0xAF,
	SET_DISP_OFF             = 0xAE,
	//Addressing settings command:
	SET_LOWER_COL_START_ADRS = 0x00,
	SET_HIGHER_COL_START_ADRS= 0x10,
	SET_MEMORY_ADDR_MODE     = 0x20,
	SET_COL_ADRS             = 0x21,
	SET_PAGE_ADRS            = 0x22,
	SET_PAGE_START_ADRS      = 0xB0,
	//HW configuration commands:
	SET_DISP_START_LINE      = 0x40,
	SET_SEG_REMAP            = 0xA0,
	SET_MLTPLX_RATIO         = 0xA8,
	SET_COM_OUTPUT_INCR      = 0xC0,
	SET_COM_OUTPUT_DECR      = 0xC8,
	SET_DISP_OFFSET          = 0xD3,
	SET_COM_PINS_HW          = 0xDA,
	ENABLE_CHARGE_PUMP_REG   = 0x8D,
#define ENABLE_CHARGE_PUMP         0x14
#define DISABLE_CHARGE_PUMP        0x10
	//Timing & Driving commands:
	SET_DISP_CLOCK_DEV       = 0xD5,
	SET_PRECHARGE_PERIOD     = 0xD9,
	SET_VCOMH_DESELECT_LVL   = 0xDB,
	NOP                      = 0xE3,
};
