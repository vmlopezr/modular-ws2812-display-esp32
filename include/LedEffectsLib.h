#ifndef LEDEFFECTSLIB_H
#define LEDEFFECTSLIB_H

#include "Esp32CtrlLed.h"
#include "Globals.h"

void HorizontalLine(Esp32CtrlLed &matrix, uint8_t rowindex, uint8_t previndex, uint8_t width, uint8_t height, uint32_t color);
void SinglePixel(uint8_t rowindex, uint8_t previndex, uint32_t color);
#endif