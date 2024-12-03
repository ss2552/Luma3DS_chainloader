/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2021 Aurora Wright, TuxSH
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include "fs.h"
#include "memory.h"
#include "fmt.h"
#include "screen.h"
#include "draw.h"
#include "utils.h"
#include "fatfs/ff.h" // DIR f_opendir FR_OK FILINFO f_readdir
#include "buttons.h"
#include "firm.h"
#include "i2c.h"

static FATFS sdFs;

static bool switchToMainDir()
{
    const char *mainDir = "/luma";

    switch(f_chdir(mainDir))
    {
        case FR_OK:
            return true;
        case FR_NO_PATH:
        {
            if (f_mkdir(mainDir) != FR_OK)
            {
                error("Failed to create luma directory.");
                return false;
            }
            return switchToMainDir();
        }
        default:
            return false;
    }
}

bool mountSdCardPartition(bool switchMainDir)
{
    static bool sdInitialized = false;
    if (!sdInitialized)
        sdInitialized = f_mount(&sdFs, "sdmc:", 1) == FR_OK;

    if (sdInitialized && switchMainDir)
        return f_chdrive("sdmc:") == FR_OK && switchToMainDir(true);
    return sdInitialized;
}

void unmountPartitions(void)
{
    f_unmount("sdmc:");
}

u32 fileRead(void *dest, const char *path, u32 maxSize)
{
    FIL file;
    FRESULT result = FR_OK;
    u32 ret = 0;

    if(f_open(&file, path, FA_READ) != FR_OK) return ret;

    u32 size = f_size(&file);
    if(dest == NULL) ret = size;
    else if(size <= maxSize)
        result = f_read(&file, dest, size, (unsigned int *)&ret);
    result |= f_close(&file);

    return result == FR_OK ? ret : 0;
}


// exceptionを消す bool fileWrite(const void *buffer, const char *path, u32 size)

bool payloadMenu(char *path, bool *hasDisplayedMenu)
{
    DIR dir;

    *hasDisplayedMenu = false;
    if(f_opendir(&dir, "ef") != FR_OK) return false;

    FILINFO info;
    u32 payloadNum = 0;
    char payloadList[20][49];

    while(f_readdir(&dir, &info) == FR_OK && info.fname[0] != 0 && payloadNum < 20)
    {
        if(info.fname[0] == '.') continue;

        u32 nameLength = strlen(info.fname);

        if(nameLength < 6 || nameLength > 52) continue;

        nameLength -= 5;

        if(memcmp(info.fname + nameLength, ".firm", 5) != 0) continue;

        memcpy(payloadList[payloadNum], info.fname, nameLength);
        payloadList[payloadNum][nameLength] = 0;
        payloadNum++;
    }

    if(f_closedir(&dir) != FR_OK || !payloadNum) return false;

    u32 pressed = 0,
        selectedPayload = 0;

    if(payloadNum != 1)
    {
        initScreens();
        *hasDisplayedMenu = true;

        drawString(true, 10, 10, COLOR_TITLE, "Luma3DS chainloader");
        drawString(true, 10, 10 + SPACING_Y, COLOR_TITLE, "Press A to select, START to quit");

        for(u32 i = 0, posY = 10 + 3 * SPACING_Y, color = COLOR_RED; i < payloadNum; i++, posY += SPACING_Y)
        {
            drawString(true, 10, posY, color, payloadList[i]);
            if(color == COLOR_RED) color = COLOR_WHITE;
        }

        while(pressed != BUTTON_A && pressed != BUTTON_START)
        {
            do
            {
                pressed = waitInput(true) & MENU_BUTTONS;
            }
            while(!pressed);

            u32 oldSelectedPayload = selectedPayload;

            switch(pressed)
            {
                case BUTTON_UP:
                    selectedPayload = !selectedPayload ? payloadNum - 1 : selectedPayload - 1;
                    break;
                case BUTTON_DOWN:
                    selectedPayload = selectedPayload == payloadNum - 1 ? 0 : selectedPayload + 1;
                    break;
                case BUTTON_LEFT:
                    selectedPayload = 0;
                    break;
                case BUTTON_RIGHT:
                    selectedPayload = payloadNum - 1;
                    break;
                default:
                    continue;
            }

            if(oldSelectedPayload == selectedPayload) continue;

            drawString(true, 10, 10 + (3 + oldSelectedPayload) * SPACING_Y, COLOR_WHITE, payloadList[oldSelectedPayload]);
            drawString(true, 10, 10 + (3 + selectedPayload) * SPACING_Y, COLOR_RED, payloadList[selectedPayload]);
        }
    }

    if(pressed != BUTTON_START)
    {
        sprintf(path, "luma/%s.firm", payloadList[selectedPayload]);

        return true;
    }

    while(HID_PAD & MENU_BUTTONS);
    wait(2000ULL);

    return false;
}