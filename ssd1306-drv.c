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

	return i2c_master_send(oled->i2c_client, oled->disp_buff,
			       DISP_BUFF_SIZE);
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
	err |= send_cmd(oled, 0x14);
	if (err) {
		LOG(KERN_DEBUG, INIT_FAULT, "Enable charge pump failed");
		return err;
	}

	LOG(KERN_DEBUG, "Driver display initialize done");

	//TODO: Testing for now, remove it later
	memset(oled->disp_buff, 0xAA, DISP_BUFF_SIZE);

	ssd1306_display(oled);

	send_cmd(oled, SET_DISP_ON);
	if (err)
		LOG(KERN_DEBUG, INIT_FAULT, "Set display ON failed");

	return err;
}
