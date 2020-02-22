#include "LedEffectsLib.h"


// extern int testValue;
// Writes the row at the newindex, and clears the row at the previous index
void HorizontalLine(uint8_t rowindex, uint8_t prevRowIndex, uint8_t width, uint8_t height, uint32_t color){
  if(rowindex < height){
    if(height == 8){
      for(int i = 0; i < width; i ++){
        matrix.setPixelRGB( prevRowIndex + i*height, 0);
        matrix.setPixelRGB( rowindex + i*height , color);
      }
    } else {
      int index = 0;
      int previndex = 0;
      for(int col = 0; col < width; col++){
        index =8*(col%8)+(rowindex%8) +  64*((int)(col/8)) + width*8*((int)(rowindex/8));
        previndex=8*(col%8)+(prevRowIndex%8) + 64*((int)(col/8)) + width*8*((int)(prevRowIndex/8));
        matrix.setPixelRGB( previndex, 0);
        matrix.setPixelRGB( index, color);
      }
    }
  }
}
void VerticalLine(uint8_t colindex, uint8_t prevColIndex, uint8_t width, uint8_t height, uint32_t color){
  if(colindex < width){
    if(width == 8){
      for(int row = 0; row < height; row ++){
        matrix.setPixelRGB( row + prevColIndex*width, 0);
        matrix.setPixelRGB( row + colindex*width , color);
      }
    } else {
      int index = 0;
      int previndex = 0;
      for(int row = 0; row < height; row++){
        index =8*(colindex%8)+(row%8) +  64*((int)(colindex/8)) + width*8*((int)(row/8));
        previndex=8*(prevColIndex%8)+(row%8) + 64*((int)(prevColIndex/8)) + width*8*((int)(row/8));
        matrix.setPixelRGB( previndex, 0);
        matrix.setPixelRGB( index, color);
      }
    }
  }
}

void SinglePixel(uint8_t rowindex, uint8_t previndex, uint32_t color){
      matrix.setPixelRGB( previndex, 0);
      matrix.setPixelRGB( rowindex, color);
}