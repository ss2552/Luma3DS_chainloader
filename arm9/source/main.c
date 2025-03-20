#include "fs.c" // mountSdCardPartition
#include "i2c.h" // I2C_init
#include "firm.h" // loadHomebrewFirm
#include "utils.h" // error mcuSetInfoLedPattern
#include "exceptions.h" // installArm9Handlers detectAndProcessExceptionDumps

extern u8 __itcm_start__[], __itcm_lma__[], __itcm_bss_start__[], __itcm_end__[];

void main(){
    
    memcpy(__itcm_start__, __itcm_lma__, __itcm_bss_start__ - __itcm_start__);
    memset(__itcm_bss_start__, 0, __itcm_end__ - __itcm_bss_start__);

    // ioの初期化
    I2C_init();

    // sdカードが読み込めれるか
    if(!mountSdCardPartition())
        error("SD mount error !!!!!");

    loadHomebrewFirm();

}