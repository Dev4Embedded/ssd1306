/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>

#define CLASS_NAME     "oled"
#define DEVICE_NAME    "ssd1306"

#define MINOR_BASE     0
#define MINOR_COUNT    1

struct ssd1306 {
	struct cdev char_dev;
	struct device *device;
	struct i2c_client *i2c_client;

	struct mutex lock;
	struct list_head list;
};

int ssd1306_init_hw(struct ssd1306 *oled);
