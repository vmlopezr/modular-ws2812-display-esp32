#include "LedEffectsLib.h"


// extern int testValue;
// Writes the row at the newindex, and clears the row at the previous index
void HorizontalLine(Esp32CtrlLed &matrix, uint8_t rowindex, uint8_t previndex, uint8_t width, uint8_t height, uint32_t color){
  if(rowindex < height){
    for(int i = 0; i < width; i ++){
      matrix.setPixelRGB( previndex + i*8, 0);
      matrix.setPixelRGB( rowindex + i*8 , color);
    }
  }
}

void SinglePixel(uint8_t rowindex, uint8_t previndex, uint32_t color){
      matrix.setPixelRGB( previndex, 0);
      matrix.setPixelRGB( rowindex, color);
}