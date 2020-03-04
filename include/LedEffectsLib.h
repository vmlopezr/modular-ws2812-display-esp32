#ifndef LEDEFFECTSLIB_H
#define LEDEFFECTSLIB_H

#include "Esp32CtrlLed.h"
#include "Globals.h"

void HorizontalLine(uint8_t rowindex, uint8_t previndex, uint8_t width, uint8_t height, uint32_t color);
void VerticalLine(uint8_t rowindex, uint8_t prevRowIndex, uint8_t width, uint8_t height, uint32_t color);
void SinglePixel(uint8_t rowindex, uint8_t previndex, uint32_t color);
void addRow(const char * Direction, uint32_t * buffer, uint32_t buffer_row);
void addColumn(const char * Direction, uint32_t * buffer, uint32_t buffer_column);
void shiftMatrixHorizontal(const char * Direction);
void shiftMatrixVertical(const char * Direction);
size_t getPixelIndex(int x, int y);
#endif