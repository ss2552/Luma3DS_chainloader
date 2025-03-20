#pragma once

#include "types.h"

u32 waitInput(bool isMenu);
void wait(u64 amount);
void error(const char *fmt, ...);

void mcuSetInfoLedPattern(u8 r, u8 g, u8 b, u32 periodMs, bool smooth);