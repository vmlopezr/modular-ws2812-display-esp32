#include "LedEffectsLib.h"


// extern int testValue;
// Writes the row at the newindex, and clears the row at the previous index
void HorizontalLine(uint8_t rowindex, uint8_t prevRowIndex, uint8_t width, uint8_t height, uint32_t color){
  if(rowindex < height){
    int index = 0;
    int previndex = 0;
    for(int col = 0; col < width; col++){
      index = getPixelIndex(col, rowindex);
      previndex = getPixelIndex(col, prevRowIndex);
      matrix.setPixelRGB(previndex, 0);
      matrix.setPixelRGB(index, color);
    }
  }
}
void VerticalLine(uint8_t colindex, uint8_t prevColIndex, uint8_t width, uint8_t height, uint32_t color){
  if(colindex < width){
    int index = 0;
    int previndex =0;
    for(int row = 0; row < height; row++){
      index = getPixelIndex(colindex,row);
      previndex=getPixelIndex(prevColIndex, row);
      matrix.setPixelRGB( previndex, 0);
      matrix.setPixelRGB( index, color);
    }
  }
}

void SinglePixel(uint8_t rowindex, uint8_t previndex, uint32_t color){
  matrix.setPixelRGB( previndex, 0);
  matrix.setPixelRGB( rowindex, color);
}
size_t getPixelIndex(int x, int y){
  if(isCJMCU == 0 && x%2 == 1){
    return 8*(x%8)+(7 - y%8) +  64*((int)(x/8)) + width*8*((int)(y/8));
  } else {
    return 8*(x%8)+(y%8) +  64*((int)(x/8)) + width*8*((int)(y/8));
  }
}