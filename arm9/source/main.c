/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2023 Aurora Wright, TuxSH
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
 書き換えたファイル
 start.s
  main.c
   fs.h
   i2c.h
   firm.h
    firm.c
     screen.h
      screen.c
   utils.h
   exceptions.h
    types.h
    exceptions.c
     
*/
#include "fs.h" // mountSdCardPartition
#include "i2c.h"
#include "firm.h" // loadHomebrewFirm
#include "utils.h" // error mcuSetInfoLedPattern
#include "exceptions.h" // installArm9Handlers detectAndProcessExceptionDumps
#include "memory.h"
extern u8 __itcm_start__[], __itcm_lma__[], __itcm_bss_start__[], __itcm_end__[];

u16 launchedPath[80+1];

u16 mcuFwVersion;
u8 mcuConsoleInfo[9];

void main(void)
{
    // Set up the additional sections, overwrites argc
    memcpy(__itcm_start__, __itcm_lma__, __itcm_bss_start__ - __itcm_start__);
    memset(__itcm_bss_start__, 0, __itcm_end__ - __itcm_bss_start__);
    I2C_init();

    u8 mcuFwVerHi = I2C_readReg(I2C_DEV_MCU, 0) - 0x10;
    u8 mcuFwVerLo = I2C_readReg(I2C_DEV_MCU, 1);
    mcuFwVersion = ((u16)mcuFwVerHi << 16) | mcuFwVerLo;

    // Check if fw is older than factory. See https://www.3dbrew.org/wiki/MCU_Services#MCU_firmware_versions for a table
    if (mcuFwVerHi < 1) error("Unsupported MCU FW version %d.%d.", (int)mcuFwVerHi, (int)mcuFwVerLo);

    I2C_readRegBuf(I2C_DEV_MCU, 0x7F, mcuConsoleInfo, 9);


    // 通知ランプ
    mcuSetInfoLedPattern(255, 0, 255, 500, false);
   
    installArm9Handlers();

    if(!mountSdCardPartition(true)) error("Failed to mount SD.");

    detectAndProcessExceptionDumps();

    // 0:/ or 0:/ef
    loadHomebrewFirm();
}

/*
start.s 引数
 main.c loadHomebrewFirm起動
  firm.h
   firm.c loadHomebrewFirm 確認 firmの実行
    screen.h
     screen.c firm選択
*/