#pragma once

#include "types.h"

bool mountSdCardPartition(void);

u32 fileRead(void *dest, const char *path, u32 maxSize);
bool payloadMenu(char *path);