/* SPDX-License-Identifier: GPL-2.0 */


#define DEFAULT_FONT_WIDTH    8
#define DEFAULT_FONT_HEIGHT   8

int ssd1306_print_char(struct ssd1306 *oled, int x, int y, char c);
int ssd1306_print_str(struct ssd1306 *oled, int x, int y, const char* str);
