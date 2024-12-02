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

/*
*   Crypto libs from http://github.com/b1l1s/ctr
*   kernel9Loader code originally adapted from https://github.com/Reisyukaku/ReiNand/blob/228c378255ba693133dec6f3368e14d386f2cde7/source/crypto.c#L233
*   decryptNusFirm code adapted from https://github.com/mid-kid/CakesForeveryWan/blob/master/source/firm.c
*   ctrNandWrite logic adapted from https://github.com/d0k3/GodMode9/blob/master/source/nand/nand.c
*/

#include "crypto.h"
#include "alignedseqmemcpy.h"

/****************************************************************
*                  Crypto libs
****************************************************************/

/* original version by megazig */

static void sha_wait_idle()
{
    while(*REG_SHA_CNT & 1);
}

void sha(void *res, const void *src, u32 size, u32 mode)
{
    sha_wait_idle();
    *REG_SHA_CNT = mode | SHA_CNT_OUTPUT_ENDIAN | SHA_NORMAL_ROUND;

    const u8 *src8 = (const u8 *)src;
    while(size >= 0x40)
    {
        sha_wait_idle();
        alignedseqmemcpy((void *)REG_SHA_INFIFO, src8, 0x40);

        src8 += 0x40;
        size -= 0x40;
    }

    sha_wait_idle();
    alignedseqmemcpy((void *)REG_SHA_INFIFO, src8, size);

    *REG_SHA_CNT = (*REG_SHA_CNT & ~SHA_NORMAL_ROUND) | SHA_FINAL_ROUND;

    while(*REG_SHA_CNT & SHA_FINAL_ROUND);
    sha_wait_idle();

    u32 hashSize = SHA_256_HASH_SIZE;
    if(mode == SHA_224_MODE)
        hashSize = SHA_224_HASH_SIZE;
    else if(mode == SHA_1_MODE)
        hashSize = SHA_1_HASH_SIZE;

    alignedseqmemcpy(res, (void *)REG_SHA_HASH, hashSize);
}