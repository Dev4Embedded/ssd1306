// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/i2c.h>

#include "ssd1306.h"

int ssd1306_init_hw(struct ssd1306 *oled)
{
	if (!oled) {
		printk(KERN_DEBUG "No access to the oled device");
		return -ENXIO;
	}

	if (!oled->i2c_client) {
		printk(KERN_DEBUG "No access to the i2c device");
		return -ENXIO;
	}

	//Check if ssd1306 was connected to the bus
	return i2c_smbus_write_byte_data(oled->i2c_client, 0x00, 00);
}
