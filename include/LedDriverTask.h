#ifndef LEDDRIVERTASK_H
#define LEDDRIVERTASK_H

#include "Globals.h"
#include "Filesystem.h"

void LedDriverTask(void *parameter);
void processLiveData();
size_t getNumericDec(const char * number);
#endif