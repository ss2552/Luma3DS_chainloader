#include "fs.h" // mountSdCardPartition
#include "i2c.h" // I2C_init
#include "firm.h" // loadHomebrewFirm
#include "utils.h" // error mcuSetInfoLedPattern
#include "memory.h" // memcpy memset
#include "exceptions.h" // installArm9Handlers detectAndProcessExceptionDumps

extern u8 __itcm_start__[], __itcm_lma__[], __itcm_bss_start__[], __itcm_end__[];

void main(int argc, char ** argv, u32 magicWord)
{
  (void)argc;
  (void)argv;

  if((magicWord & 0xFFFF) == 0xBEEF)mcuSetInfoLedPattern(255, 0, 0, 6000, false);

  memcpy(__itcm_start__, __itcm_lma__, __itcm_bss_start__ - __itcm_start__);
  memset(__itcm_bss_start__, 0, __itcm_end__ - __itcm_bss_start__);

  I2C_init();

  installArm9Handlers();

  if(!mountSdCardPartition()) error("main.c : SD!!!!!");

  detectAndProcessExceptionDumps();
  
  loadHomebrewFirm();
}
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

/*
start.s main
main.c loadHomebrewFirm
firm.c payloadMenu
  fs.c firmの選択
firm.c launchFirm chainload
start.s chainloader_main
chainloader.c doLaunchFirm disableMpuAndJumpToEntrypoints
start.c r6(arm9Entry)

static Firm *firm = (Firm *)0x20001000;にfirmを書き込んでr6で実行されている

ーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーーー


*/
/*
mcuSetInfoLedPattern


I2C_writeReg
arm9_exception_handlers
113 0x22
117 0x20
screen
109 0x22
*/