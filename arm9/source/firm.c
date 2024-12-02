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

#include "firm.h"
#include "utils.h"
#include "fs.h"
#include "exceptions.h"
#include "memory.h"
#include "cache.h"
#include "crypto.h"
#include "screen.h"
#include "fmt.h"
#include "chainloader.h"

static Firm *firm = (Firm *)0x20001000;

static __attribute__((noinline)) bool overlaps(u32 as, u32 ae, u32 bs, u32 be)
{
    if(as <= bs && bs <= ae)
        return true;
    if(bs <= as && as <= be)
        return true;
    return false;
}

static __attribute__((noinline)) bool inRange(u32 as, u32 ae, u32 bs, u32 be)
{
   if(as >= bs && ae <= be)
        return true;
   return false;
}

static bool checkFirm(u32 firmSize)
{
    if(memcmp(firm->magic, "FIRM", 4) != 0 || firm->arm9Entry == NULL) //Allow for the Arm11 entrypoint to be zero in which case nothing is done on the Arm11 side
        return false;

    bool arm9EpFound = false,
         arm11EpFound = false;

    u32 size = 0x200;
    for(u32 i = 0; i < 4; i++)
        size += firm->section[i].size;

    if(firmSize < size) return false;

    for(u32 i = 0; i < 4; i++)
    {
        FirmSection *section = &firm->section[i];

        //Allow empty sections
        if(section->size == 0)
            continue;

        if((section->offset < 0x200) ||
           (section->address + section->size < section->address) || //Overflow check
           ((u32)section->address & 3) || (section->offset & 0x1FF) || (section->size & 0x1FF) || //Alignment check
           (overlaps((u32)section->address, (u32)section->address + section->size, (u32)firm, (u32)firm + size)) ||
           ((!inRange((u32)section->address, (u32)section->address + section->size, 0x08000000, 0x08000000 + 0x00100000)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x18000000, 0x18000000 + 0x00600000)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x1FF00000, 0x1FFFFC00)) &&
            (!inRange((u32)section->address, (u32)section->address + section->size, 0x20000000, 0x20000000 + 0x8000000))))
            return false;

        __attribute__((aligned(4))) u8 hash[0x20];

        sha(hash, (u8 *)firm + section->offset, section->size, SHA_256_MODE);

        if(memcmp(hash, section->hash, 0x20) != 0)
            return false;

        if(firm->arm9Entry >= section->address && firm->arm9Entry < (section->address + section->size))
            arm9EpFound = true;

        if(firm->arm11Entry >= section->address && firm->arm11Entry < (section->address + section->size))
            arm11EpFound = true;
    }

    return arm9EpFound && (firm->arm11Entry == NULL || arm11EpFound);
}

void launchFirm(int argc, char **argv)
{
    prepareArm11ForFirmlaunch();
    chainload(argc, argv, firm);
}

void loadHomebrewFirm()
{
    char path[10 + 255];
    bool hasDisplayedMenu = false;
    
    if(!payloadMenu(path, &hasDisplayedMenu)) return;

    u32 maxPayloadSize = (u32)((u8 *)0x27FFE000 - (u8 *)firm);
    u32 payloadSize = fileRead(firm, path, maxPayloadSize);

    if(payloadSize <= 0x200 || !checkFirm(payloadSize)) error("The payload is invalid or corrupted.");

    char absPath[24 + 255];

    sprintf(absPath, "sdmc:/luma/%s", path);

    char *argv[2] = {absPath, (char *)fbs};
    bool wantsScreenInit = (firm->reserved2[0] & 1) != 0;

    if(!hasDisplayedMenu && wantsScreenInit)
        initScreens(); // Don't init the screens unless we have to, if not already done

    launchFirm(wantsScreenInit ? 2 : 1, argv);
}