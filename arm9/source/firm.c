#include "firm.h"
#include "fs.h"
#include "screen.h"
#include "chainloader.h"
#include "utils.h"
#include "fmt.h"

static Firm *firm = (Firm *)0x20001000;

void launchFirm(int argc, char **argv)
{
    prepareArm11ForFirmlaunch();
    chainload(argc, argv, firm);
}

void loadHomebrewFirm()
{
    char path[10 + 255];
    
    if(!payloadMenu(path)) return;

    u32 maxPayloadSize = (u32)((u8 *)0x27FFE000 - (u8 *)firm);
    u32 payloadSize = fileRead(firm, path, maxPayloadSize);

    if(payloadSize <= 0x200) error("The payload is invalid or corrupted.");
    
    char absPath[24 + 255];

    sprintf(absPath, "sdmc:/luma/%s", path);

    char *argv[2] = {absPath, (char *)fbs};
    bool wantsScreenInit = (firm->reserved2[0] & 1) != 0;
        
    launchFirm(wantsScreenInit ? 2 : 1, argv);
}