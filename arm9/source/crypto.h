/*
*   This file is part of Luma3DS
*   Copyright (C) 2016-2020 Aurora Wright, TuxSH
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
*   Crypto libs from http://github.com/b1l1s/ctr
*   kernel9Loader code originally adapted from https://github.com/Reisyukaku/ReiNand/blob/228c378255ba693133dec6f3368e14d386f2cde7/source/crypto.c#L233
*   decryptNusFirm code adapted from https://github.com/mid-kid/CakesForeveryWan/blob/master/source/firm.c
*   ctrNandWrite logic adapted from https://github.com/d0k3/GodMode9/blob/master/source/nand/nand.c
*/

#pragma once

#include "types.h"

/**************************SHA****************************/
#define REG_SHA_CNT         ((vu32 *)0x1000A000)
#define REG_SHA_HASH        ((vu32 *)0x1000A040)
#define REG_SHA_INFIFO      ((vu32 *)0x1000A080)

#define SHA_CNT_OUTPUT_ENDIAN   0x00000008
#define SHA_HASH_READY      0x00000000
#define SHA_NORMAL_ROUND    0x00000001
#define SHA_FINAL_ROUND     0x00000002

#define SHA_256_MODE        0
#define SHA_224_MODE        0x00000010
#define SHA_1_MODE          0x00000020

#define SHA_256_HASH_SIZE   (256 / 8)
#define SHA_224_HASH_SIZE   (224 / 8)
#define SHA_1_HASH_SIZE     (160 / 8)

void sha(void *res, const void *src, u32 size, u32 mode);