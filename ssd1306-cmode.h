/* SPDX-License-Identifier: GPL-2.0 */

int ssd1306_cmode_setup(struct ssd1306_cmode *cmode, int fonth, int fontv,
			int resh, int resv);
void ssd1306_cmode_free(struct ssd1306_cmode *cmode);
int ssd1306_cut_str(struct ssd1306_cmode* cmode, char* str);
