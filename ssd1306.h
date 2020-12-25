/* SPDX-License-Identifier: GPL-2.0 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>

#include "ssd1306-cmds.h"

#define CLASS_NAME     "oled"
#define DEVICE_NAME    "ssd1306"

#define MINOR_BASE     0
#define MINOR_COUNT    1

/**
 * TODO: These values should be taken from device tree
 */
#define SSD1306_VERTICAL_MAX 32
#define SSD1306_HORIZONTAL_MAX 128
#define SSD1306_CELL_CAPACITY 8


#define LOG(sev, ...) printk(sev "ssd1306: " __VA_ARGS__)
/**
 * TODO: Currently hard-code buffer size for 128x32 resolution.
 *       In next steps extend functionality to get resolution from DTB.
 *
 *       Increment buffer size +1 byte for extra command sending during refresh
 *       transmission.
*/
#define DISP_BUFF_SIZE    (512 + 1)

struct ssd1306_cmode{
	int max_cols;       /*! Max. characters in single line */
	int max_lines;      /*! Max. lines on the display */
	int max_buff_size;  /*! Max. display capacity */
	char **actual_disp; /*! Array contains actually displaying strings */
};

struct ssd1306 {
	struct cdev char_dev;
	struct device *device;
	struct i2c_client *i2c_client;
	struct ssd1306_cmode cmode;
	uint8_t *disp_buff;

};

int ssd1306_init_hw(struct ssd1306 *oled);
void ssd1306_deinit_hw(struct ssd1306 *oled);
int ssd1306_display(struct ssd1306 *oled);
int ssd1306_clear_display(struct ssd1306 *oled);
int ssd1306_draw_pxl(struct ssd1306 *oled, int x, int y);
int ssd1306_enable_charge_pump(struct ssd1306* oled, bool enable);
int ssd1306_enable_display(struct ssd1306* oled, bool enable);
