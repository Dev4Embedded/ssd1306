// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/i2c.h>

#include "ssd1306.h"

#define SSD1306_LEN        0x3
#define SSD1306_ADDRESS    0x3C
#define SSD1306_CONTROL    0x00

static int send_cmd(struct ssd1306 *oled, enum ssd1306_cmd cmd)
{
	if (!oled) {
		printk(KERN_DEBUG "No access to the oled device\n");
		return -ENXIO;
	}

	if (!oled->i2c_client) {
		printk(KERN_DEBUG "No access to the i2c device\n");
		return -ENXIO;
	}

	printk(KERN_DEBUG "SSD1306: Send commmand 0x%.2X\n", cmd);

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
	if (!oled)
		return -EPERM;

	if (oled->disp_buff[0] != SET_DISP_START_LINE) {
		printk(KERN_DEBUG "Display buffer infected\n");
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
	int err;

	//Check if ssd1306 was connected to the bus
	err = send_cmd(oled, NOP);
	if (err) {
		printk(KERN_DEBUG "Cannot connect to SSD1306 display\n");
		return -EIO;
	}

	send_cmd(oled, SET_DISP_OFF);

	send_cmd(oled, SET_MLTPLX_RATIO);
	send_cmd(oled, 0x3F);

	send_cmd(oled, SET_DISP_OFFSET);
	send_cmd(oled, 0);

	send_cmd(oled, SET_DISP_START_LINE);

	send_cmd(oled, SET_SEG_REMAP);

	send_cmd(oled, SET_COM_OUTPUT_INCR);

	send_cmd(oled, SET_COM_PINS_HW);
	send_cmd(oled, 0x02);

	send_cmd(oled, SET_CONTRAST_CTRL);
	send_cmd(oled, 0xFF);

	send_cmd(oled, ENTIRE_DISP_ON);

	send_cmd(oled, SET_DISP_CLOCK_DEV);
	send_cmd(oled, 0x80);

	send_cmd(oled, ENABLE_CHARGE_PUMP_REG);
	send_cmd(oled, 0x14);

	printk(KERN_DEBUG "Init done");

	send_cmd(oled, SET_MEMORY_ADDR_MODE);
	send_cmd(oled, 0x00);

	send_cmd(oled, SET_COL_ADRS);
	send_cmd(oled, 0x00);
	send_cmd(oled, 127);

	send_cmd(oled, SET_PAGE_ADRS);
	send_cmd(oled, 0x00);
	send_cmd(oled, 7);

	//TODO: Testing for now, remove it later
	memset(oled->disp_buff, 0xAA, DISP_BUFF_SIZE);

	ssd1306_display(oled);

	send_cmd(oled, SET_DISP_ON);
	return 0;
}
