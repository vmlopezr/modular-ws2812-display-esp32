#ifndef WS2812SDCARD_H
#define WE2812SDCARD_H

#include "Esp32CtrlLed.h"
#include "SD.h"
#include "FS.h"
#include "Globals.h"

void loadDataFromStorage(Esp32CtrlLed &Leds, const char *filename);
void loadDataFromWireless(uint8_t *buffer, const char* filename);


#endif