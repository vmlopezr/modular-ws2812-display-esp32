#ifndef WS2812SDCARD_H
#define WE2812SDCARD_H

#include "Esp32CtrlLed.h"
#include "SD.h"
#include "FS.h"

void loadDataFromStorage(fs::FS &fs, Esp32CtrlLed &Leds, const char *filename, size_t height, size_t width);
void loadDataFromWireless();


#endif