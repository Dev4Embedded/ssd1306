// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/font.h>
#include <linux/slab.h>

#include "ssd1306.h"

#define DEFAULT_FONT_WIDTH    8
#define DEFAULT_FONT_HEIGHT   8

/**
 * @brief
 *     Draw single ASCII character using default kernel font
 *
 * @param[IN] oled    pointer to SSD1306 main handle
 * @param[IN] x       start of horizontal coordinate
 * @param[IN] y       start of vertical coordinate
 * @param[IN] c       ASCII character
 *
 * @return returns zero or negative error
 */
int ssd1306_print_char(struct ssd1306 *oled, int x, int y, char c)
{
	const struct font_desc *font = NULL;
	int line_x, line_y;
	uint8_t vline;
	uint8_t *ptr;

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

	/* TODO: Get default character for now. Give user possibility
	 *       to choose in future
	 */
	font = get_default_font(SSD1306_HORIZONTAL_MAX, SSD1306_VERTICAL_MAX,
				DEFAULT_FONT_WIDTH, DEFAULT_FONT_HEIGHT);

	if (!font) {
		LOG(KERN_DEBUG, "Given font does not exist");
		return -EPERM;
	}

	LOG(KERN_DEBUG, "Default font name: %s, Character: %c,"
	    " Array position: %d", font->name, c, (c * DEFAULT_FONT_HEIGHT));

	/* Start address of given character in font array
	 * TODO: Remember to change DEFAULT_FONT_HEIGHT to the font height
	 *       when the font selection option is provided
	 */
	ptr = (uint8_t *) font->data + (c * DEFAULT_FONT_HEIGHT);

	for (line_y=0; line_y<DEFAULT_FONT_HEIGHT; line_y++)
	{
		memcpy(&vline, (ptr + line_y), sizeof(vline));

		for (line_x = 0; line_x < DEFAULT_FONT_WIDTH; line_x++) {
			if (vline & 0x80)
				/* Ignore errors, out of margin it's allowed*/
				ssd1306_draw_pxl(oled, x + line_x, y + line_y);
			vline <<= 1;
		}
	}

	return 0;
}
