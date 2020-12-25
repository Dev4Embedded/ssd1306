// SPDX-License-Identifier: (GPL-2.0 OR MIT)

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "ssd1306.h"
#include "ssd1306-cmode.h"

#define ALFANUM(character) (character >= 0x20 && character <= 0x7E ? 1 : 0)

/**
 * @brief
 *     Setup character mode for SSD1306 display. Calculate available space for
 *     on the display, in other words: how many characters user can put on
 *     the display. Set responsible variables and allocate needed space for
 *     buffer
 *
 * @param[IN] cmode    pointer to character mode structure
 * @param[IN] fonth    uses font width
 * @param[IN] fontv    uses font height
 * @param[IN] resh     maximum display width
 * @param[IN] resv     maximum display height
 *
 * @return returns zero or negative error
 */
int ssd1306_cmode_setup(struct ssd1306_cmode *cmode, int fonth, int fontv,
			int resh, int resv)
{
	int line;

	if (!cmode)
		return -EINVAL;

	if (fontv > resv) {
		LOG(KERN_DEBUG, "Bad configuration: font height is larger than"
		    " the display");
		return -EINVAL;
	}

	if (fonth > resh)
	{
		LOG(KERN_DEBUG, "Bad configuration: font width is larger than"
		    " the display");
		return -EINVAL;
	}

	//Hard-coded +1 pixel between characters:
	cmode->max_cols = resh / (fonth + 1);
	cmode->max_lines = resv / fontv;
	cmode->max_buff_size = cmode->max_cols * cmode->max_lines;
	cmode->actual_disp = (char **)kmalloc(cmode->max_lines * sizeof(char *),
					      GFP_KERNEL);
	memset(cmode->actual_disp, 0, cmode->max_lines);

	if (!cmode->actual_disp)
		goto exit;

	for (line = 0; line < cmode->max_lines; line++) {
		//Allocate +1 extra character for string line end signaling
		const int length = cmode->max_cols + 1;

		cmode->actual_disp[line] = (char *)kmalloc(length, GFP_KERNEL);

		if (!cmode->actual_disp[line]) {
			int alloc_line = line - 1;

			for (; alloc_line >= 0; alloc_line--)
				kfree(cmode->actual_disp[alloc_line]);

			goto exit;
		}

		memset(cmode->actual_disp[line], 0, length);
	}

	return 0;
exit:
	if (cmode->actual_disp)
		kfree(cmode->actual_disp);

	LOG(KERN_WARNING, "Cannot allocate enough space for character display"
	    " buffer");

	return -ENOMEM;
}

/**
 * @brief
 *     Frees allocated resources
 *
 * @param[IN] cmode    pointer to character mode structure
 *
 */
void ssd1306_cmode_free(struct ssd1306_cmode *cmode)
{
	int line;

	for (line = 0; line < cmode->max_lines; line++) {
		kfree(cmode->actual_disp[line]);
	}

	kfree(cmode->actual_disp);

	cmode->actual_disp = NULL;
}

/**
 * @brief
 *     Cut user character string and copy to related display lines buffers
 *
 * @param[IN] cmode    pointer to character mode structure
 * @param[IN] str      string to cut
 *
 * @return returns number of copied characters or negative error
 */
int ssd1306_cut_str(struct ssd1306_cmode* cmode, char* str)
{
	char *position;
	char *line;
	int col_num;
	int counter = 0;
	int line_num = 0;

	if (!str || !cmode)
		return -EINVAL;

	position = str;

	for (; line_num < cmode->max_lines; line_num++) {
		line = cmode->actual_disp[line_num];

		for(col_num = 0; col_num < cmode->max_cols; col_num++) {
			if (position) {

				/** The string could contains special characters
				 *  like new line or carrier return and others
				 *  unsupported by font. Do not copy it.
				 *  Skip one line.
				 */
				if (!ALFANUM(*position)) {
					while (col_num < cmode->max_cols)
						line[col_num++] = 0;
					break;
				} else {
					//Copy the valid character
					line[col_num] = *position;
					counter++;
				}
				position++;
			} else {
				//End of string, so clean rest of lines buffers
				line[col_num] = 0;
			}
		}
	}

	return counter;
}
