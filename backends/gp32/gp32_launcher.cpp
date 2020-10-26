/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include "base/engine.h"

#include "backends/gp32/gp32std.h"
#include "backends/gp32/gp32std_grap.h"
#include "backends/gp32/gp32std_input.h"

#include "backends/gp32/gfx_splash.h"
#include "backends/gp32/gp32_launcher.h"

uint16 cpuSpeedTable[15] = {40, 66, 100, 120, 133, 144, 156, 160, 166, 172, 176, 180, 188, 192, 200};
uint16 gammaTable[16] = {50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200};
char *oplTable[3] = {"LOW", "MEDIUM", "HIGH"};
uint16 sampleTable[3] = {11025, 22050, 44100};

uint8 maxTable[5] = {15, 16, 3, 3, 2};
uint8 currentSetting[5] = {2, 5, 1, 0, 0};

int g_cpuSpeed = 100;
int g_gammaRamp = 100;

void configMenu() {
	uint32 nKeyUD;
	uint16 nKeyP;

	int currentSelect = 0;
	char text[32];

	while (1) {
		gp_fillRect(frameBuffer2, 0, 0, 320, 240, 0xffff);
		gp_textOut(frameBuffer2, 90, 10, "Configuration Menu", 0);

		gp_textOut(frameBuffer2, 30, 40, "CPU clock speed", 0);
		gp_textOut(frameBuffer2, 30, 80, "Gamma ramp", 0);
		gp_textOut(frameBuffer2, 30, 120, "FMOPL (AdLib) quality", gp_RGBTo16(128, 128, 128));
		gp_textOut(frameBuffer2, 30, 160, "Sampling rate", gp_RGBTo16(128, 128, 128));

		gp_textOut(frameBuffer2, 100, 210, "OK         CANCEL", 0);

		if (currentSelect == 4)
			gp_textOut(frameBuffer2, 80, 210, "@", 0);
		else
			gp_textOut(frameBuffer2, 20, (currentSelect + 1) * 40, "@", 0);

		sprintf(text, "%d MHz", cpuSpeedTable[currentSetting[0]]);
		gp_textOut(frameBuffer2, 220, 40, text, 0);
		sprintf(text, "%d %%", gammaTable[currentSetting[1]]);
		gp_textOut(frameBuffer2, 220, 80, text, 0);
		gp_textOut(frameBuffer2, 220, 120, oplTable[currentSetting[2]], gp_RGBTo16(128, 128, 128));
		sprintf(text, "%d Hz", sampleTable[currentSetting[3]]);
		gp_textOut(frameBuffer2, 220, 160, text, gp_RGBTo16(128, 128, 128));

		gp_flipScreen();

		gp_getButtonEvent(&nKeyUD, &nKeyP);

		if (gpd_getButtonDown(nKeyUD, GPC_VK_UP)) {
			if (currentSelect > 0)
				currentSelect--;
		}
		if (gpd_getButtonDown(nKeyUD, GPC_VK_DOWN)) {
			if (currentSelect < 4)
				currentSelect++;
		}
		if (gpd_getButtonDown(nKeyUD, GPC_VK_LEFT)) {
			if (currentSelect <= 1)
				if (currentSetting[currentSelect] > 0)
					currentSetting[currentSelect]--;
		}
		if (gpd_getButtonDown(nKeyUD, GPC_VK_RIGHT)) {
			if (currentSelect <= 1)
				if (currentSetting[currentSelect] < maxTable[currentSelect] - 1)
					currentSetting[currentSelect]++;
		}

		if (gpd_getButtonUp(nKeyUD, GPC_VK_START) ||
			gpd_getButtonUp(nKeyUD, GPC_VK_FA)) {
			if (currentSelect == 4) {
				if (currentSetting[currentSelect] == 0) { // OK
					g_cpuSpeed = cpuSpeedTable[currentSetting[0]];
					g_gammaRamp = gammaTable[currentSetting[1]];
					return;
				} else { // CANCEL
					return;
				}
			}
		}
	}
}

void splashScreen() {
	uint32 nKeyUD;
	uint16 nKeyP;

	while (1) {
		uint16 *buffer = frameBuffer2;//&frameBuffer1[240 - _screenHeight];
		uint8 *picBuffer = gfx_splash;

		for (int c = 0; c < LCD_WIDTH * LCD_HEIGHT; c++) {
			*buffer++ = gfx_splash_Pal[*picBuffer++];
		}

		gp_flipScreen();

		while (1) {
			gp_getButtonEvent(&nKeyUD, &nKeyP);

			if (gpd_getButtonUp(nKeyUD, GPC_VK_START) ||
				gpd_getButtonUp(nKeyUD, GPC_VK_FA)) {
				gp_fillRect(frameBuffer1, 0, 0, 320, 240, 0xffff);
				gp_fillRect(frameBuffer2, 0, 0, 320, 240, 0xffff);
				return;
			}
			if (gpd_getButtonUp(nKeyUD, GPC_VK_SELECT)) {
				configMenu();
				break;
			}
		}
	}
}
