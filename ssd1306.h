/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>

#include "ssd1306-cmds.h"

#define CLASS_NAME     "oled"
#define DEVICE_NAME    "ssd1306"

#define MINOR_BASE     0
#define MINOR_COUNT    1

#define LOG(sev, ...) printk(sev "ssd1306: " __VA_ARGS__)
/**
 * TODO: Currently hard-code buffer size for 128x32 resolution.
 *       In next steps extend functionality to get resolution from DTB.
 *
 *       Increment buffer size +1 byte for extra command sending during refresh
 *       transmission.
*/
#define DISP_BUFF_SIZE    (512 + 1)

struct ssd1306 {
	struct cdev char_dev;
	struct device *device;
	struct i2c_client *i2c_client;

	struct mutex lock;
	struct list_head list;

	uint8_t *disp_buff;
};

int ssd1306_init_hw(struct ssd1306 *oled);
int ssd1306_display(struct ssd1306 *oled);
