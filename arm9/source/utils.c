/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2022 Aurora Wright, TuxSH
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

/*
*   waitInput function based on code by d0k3 https://github.com/d0k3/Decrypt9WIP/blob/master/source/hid.c
*/

#include "utils.h"
#include "i2c.h"
#include "buttons.h"
#include "screen.h"
#include "draw.h"
#include "fmt.h"
#include "memory.h"
#include "fs.h"

typedef struct McuInfoLedPattern {
    u8 delay;
    u8 smoothing;
    u8 loopDelay;
    u8 reserved;
    u8 red[32];
    u8 green[32];
    u8 blue[32];
} McuInfoLedPattern;
_Static_assert(sizeof(McuInfoLedPattern) == 100, "McuInfoLedPattern: wrong size");

u32 waitInput(bool isMenu)
{
    static u64 dPadDelay = 0ULL;
    u64 initialValue = 0ULL;
    u32 key,
        oldKey = HID_PAD;
    bool shouldShellShutdown = true;

    if(isMenu)
    {
        dPadDelay = dPadDelay > 0ULL ? 87ULL : 143ULL;
        startChrono();
        initialValue = chrono();
    }

    while(true)
    {
        key = HID_PAD;

        if(!key)
        {
            if(shouldShellShutdown)
            {
                u8 shellState = I2C_readReg(I2C_DEV_MCU, 0xF);
                wait(5);
                if(!(shellState & 2)) mcuPowerOff();
            }

            u8 intStatus = I2C_readReg(I2C_DEV_MCU, 0x10);
            wait(5);
            if(intStatus & 1) mcuPowerOff(); //Power button pressed

            oldKey = 0;
            dPadDelay = 0;
            continue;
        }

        if(key == oldKey && (!isMenu || (!(key & DPAD_BUTTONS) || chrono() - initialValue < dPadDelay))) continue;

        //Make sure the key is pressed
        u32 i;
        for(i = 0; i < 0x13000 && key == HID_PAD; i++);
        if(i == 0x13000) break;
    }

    return key;
}

void wait(u64 amount)
{
    startChrono();

    u64 initialValue = chrono();

    while(chrono() - initialValue < amount);
}

void error(const char *fmt, ...)
{
    char buf[DRAW_MAX_FORMATTED_STRING_SIZE + 1];

    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    initScreens();
    drawString(true, 10, 10, COLOR_RED, "An error has occurred:");
    u32 posY = drawString(true, 10, 30, COLOR_WHITE, buf);
    drawString(true, 10, posY + 2 * SPACING_Y, COLOR_WHITE, "Press any button to shutdown");

    waitInput(false);

    mcuPowerOff();

}

static inline u8 mcuPeriodMsToTick(u32 periodMs)
{
    // 512Hz
    u32 res = 512u * periodMs / 1000u;
    res = res < 2 ? 1 : res - 1; // res not allowed to be zero
    res = res > 255 ? 255 : res; // res can't exceed 255
    return (u8)res;
}

void mcuSetInfoLedPattern(u8 r, u8 g, u8 b, u32 periodMs, bool smooth)
{
    McuInfoLedPattern pattern;

    if (periodMs == 0)
    {
        pattern.delay = 0xFF; // as high as we can; needs to be non-zero. Delay between frames (array elems)
        pattern.smoothing = 1;
        pattern.loopDelay = 0xFE; // as high as we can
        for (u32 i = 0; i < 32; i++)
        {
            pattern.red[i] = r;
            pattern.green[i] = g;
            pattern.blue[i] = b;
        }
    }
    else
    {
        periodMs = periodMs < 63 ? 63 : periodMs;
        pattern.delay = mcuPeriodMsToTick(periodMs);
        pattern.smoothing = smooth ? mcuPeriodMsToTick(periodMs) : 1;
        pattern.loopDelay = 0; // restart immediately
        for (u32 i = 0; i < 16; i++)
        {
            pattern.red[2*i] = r;
            pattern.red[2*i + 1] = 0;
            pattern.green[2*i] = g;
            pattern.green[2*i + 1] = 0;
            pattern.blue[2*i] = b;
            pattern.blue[2*i + 1] = 0;
        }
    }

    I2C_writeRegBuf(I2C_DEV_MCU, 0x2D, (const u8 *)&pattern, sizeof(McuInfoLedPattern));
}