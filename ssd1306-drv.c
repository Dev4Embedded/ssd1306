// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/i2c.h>

#include "ssd1306.h"

#define SSD1306_LEN        0x3
#define SSD1306_ADDRESS    0x3C
#define SSD1306_CONTROL    0x00

static int send_cmd(struct ssd1306 *oled, enum ssd1306_cmd cmd)
{
	if (!oled) {
		LOG(KERN_DEBUG, "No access to the oled device");
		return -ENXIO;
	}

	if (!oled->i2c_client) {
		LOG(KERN_DEBUG, "No access to the i2c device");
		return -ENXIO;
	}

	LOG(KERN_INFO, "Send command 0x%.2X", (u8)cmd);

	return i2c_smbus_write_byte_data(oled->i2c_client, 0x00, (u8)cmd);
}

/**
 * @brief
 *     Place a single pixel at the x and y coordinates
 * @note
 *     This function modify only display buffer, to perform your change
 *     use ssd1306_display() after that.
 * @param[IN]    oled    pointer to SSD1306 main handle
 * @param[IN]    y       vertical coordinate
 * @param[IN]    x       horizontal coordinate
 *
 * @return returns zero or negative error
 */
int ssd1306_draw_pxl(struct ssd1306 *oled, int x, int y)
{
	int cell_addr;
	int row;
	uint8_t bit;

	if (!oled)
		return -EPERM;

	if ( x < 0 || y < 0) {
		LOG(KERN_DEBUG, "Coordinates x and y must be grater then zero");
		return -EPERM;
	}

	if (x >= SSD1306_HORIZONTAL_MAX) {
		LOG(KERN_DEBUG, "Coordinate x has to be smaller then %d",
		    SSD1306_HORIZONTAL_MAX);
		return -EPERM;
	}

	if (y >= SSD1306_VERTICAL_MAX) {
		LOG(KERN_DEBUG, "Coordinate y has to be smaller then %d",
		    SSD1306_VERTICAL_MAX);
		return -EPERM;
	}

	row = y / SSD1306_CELL_CAPACITY;
	cell_addr = x + row * SSD1306_HORIZONTAL_MAX;
	bit = (1 << y%SSD1306_CELL_CAPACITY);

	//Should never happen in theory
	if (cell_addr >= DISP_BUFF_SIZE) {
		LOG(KERN_ALERT, "Wrong resolution provided");
		return -ERANGE;
	}

	oled->disp_buff[cell_addr] |= bit;

	return 0;
}

/**
 * @brief
 *     Send buffer content to the driver
 *
 * @param[IN] oled    pointer to SSD1306 main handle
 *
 * @return returns zero or negative error
 */
int ssd1306_display(struct ssd1306 *oled)
{
	int err;

	if (!oled)
		return -EPERM;

	err = send_cmd(oled, SET_MEMORY_ADDR_MODE);
	err |= send_cmd(oled, 0x00);
	if (err) {
		LOG(KERN_DEBUG, "Reset memory address mode failed");
		return err;
	}

	err = send_cmd(oled, SET_COL_ADRS);
	err |= send_cmd(oled, 0x00);
	err |= send_cmd(oled, 127);
	if (err) {
		LOG(KERN_DEBUG, "Set column address failed");
		return err;
	}

	err = send_cmd(oled, SET_PAGE_ADRS);
	err |= send_cmd(oled, 0x00);
	err |= send_cmd(oled, 7);
	if (err) {
		LOG(KERN_DEBUG, "Set page address failed");
		return err;
	}

	if (oled->disp_buff[0] != SET_DISP_START_LINE) {
		LOG(KERN_DEBUG, "Display buffer contaminated");
		oled->disp_buff[0] = SET_DISP_START_LINE;
	}

	err = i2c_master_send(oled->i2c_client, oled->disp_buff,
			       DISP_BUFF_SIZE);
	if (err < 0) {
		LOG(KERN_DEBUG, "Display refresh failure");
		return err;
	}

	if (err != DISP_BUFF_SIZE) {
		LOG(KERN_DEBUG, "Display refreshed incompletely");
	}

	return 0;
}

/**
 * @brief
 *     Send sequence of initial commands to the SSD1306 driver.
 *
 * @param[IN] oled    pointer to SSD1306 main handle
 *
 * @return returns zero or negative error
 */
int ssd1306_init_hw(struct ssd1306 *oled)
{
#define INIT_FAULT "Initialization fault: %s"
	int err;

	//Check if ssd1306 was connected to the bus
	err = send_cmd(oled, NOP);
	if (err) {
		LOG(KERN_DEBUG, "Cannot connect to SSD1306 display");
		return -EIO;
	}

	//Let's perform default initialization
	err = send_cmd(oled, SET_DISP_OFF);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set display OFF failed");
		return err;
	}

	err = send_cmd(oled, SET_MLTPLX_RATIO);
	err |= send_cmd(oled, 0x3F);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set multiplex ratio failed");
		return err;
	}

	err = send_cmd(oled, SET_DISP_OFFSET);
	err |= send_cmd(oled, 0);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set display offset failed");
		return err;
	}

	err = send_cmd(oled, SET_DISP_START_LINE);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set start line failed");
		return err;
	}

	err = send_cmd(oled, SET_SEG_REMAP);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set segment re-map failed");
		return err;
	}

	err = send_cmd(oled, SET_COM_OUTPUT_INCR);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set scan direction failed");
		return err;
	}

	err = send_cmd(oled, SET_COM_PINS_HW);
	err |=send_cmd(oled, 0x02);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set COM pins HW conf. failed");
		return err;
	}

	err = send_cmd(oled, SET_CONTRAST_CTRL);
	err |= send_cmd(oled, 0xFF);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set contrast control failed");
		return err;
	}

	err = send_cmd(oled, ENTIRE_DISP_ON);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set enable to RAM display failed");
		return err;
	}

	err = send_cmd(oled, SET_DISP_CLOCK_DEV);
	err |= send_cmd(oled, 0x80);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Set display clock divider failed");
		return err;
	}

	err = send_cmd(oled, ENABLE_CHARGE_PUMP_REG);
	err |= send_cmd(oled, ENABLE_CHARGE_PUMP);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Enable charge pump failed");
		return err;
	}

	LOG(KERN_DEBUG, "Driver display initialize done");

	send_cmd(oled, SET_DISP_ON);
	if (err)
		LOG(KERN_DEBUG, INIT_FAULT, "Set display ON failed");

	return err;
}

/**
 * @brief
 *     Enable or disable internal charge pump in the oled device.
 *     It is necessary to properly turn on display.
 *
 * @param[IN] oled      pointer to SSD1306 main handle
 * @param[IN] enable    enable(true) or disable(false) charge pump
 *
 * @return returns zero or negative error
 */
int ssd1306_enable_charge_pump(struct ssd1306* oled, bool enable)
{
	int err;

	if (IS_ERR_OR_NULL(oled))
		return -EPERM;

	err = send_cmd(oled, ENABLE_CHARGE_PUMP_REG);
	if (enable)
		err |= send_cmd(oled, ENABLE_CHARGE_PUMP);
	else
		err |= send_cmd(oled, DISABLE_CHARGE_PUMP);

	return err;
}

/**
 * @brief
 *     Perform transition to standby or turn on state.
 *
 * @param[IN] oled      pointer to SSD1306 main handle
 * @param[IN] enable    turn on(true) or turn off(false) display
 *
 * @return returns zero or negarive error
 */
int ssd1306_enable_display(struct ssd1306* oled, bool enable)
{
	if (IS_ERR_OR_NULL(oled))
		return -EPERM;

	if (enable)
		return send_cmd(oled, SET_DISP_ON);

	return send_cmd(oled, SET_DISP_OFF);
}
