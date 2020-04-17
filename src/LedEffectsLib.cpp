#include "LedEffectsLib.h"

// Writes the row at the newindex, and clears the row at the previous index

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
void addColumn(const char * Direction, uint32_t * buffer, uint32_t buffer_column){
  if(!strcmp("Right", Direction)){
    size_t matrix_index = 0;
    size_t buffer_index = 0;

    for(int row = 0; row < height; row++){
      matrix_index = getPixelIndex(0, row);  // The rightmost column in the matrix array is width - 1;
      buffer_index = getPixelIndex(width - 1 - buffer_column,row);   // The left column of the buffer needs to be added;
      matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
    }
    // Shifting towards the left
  } else {
    size_t matrix_index = 0;
    size_t buffer_index = 0;
    for(int row = 0; row < height; row++){
      matrix_index = getPixelIndex(width - 1, row); // The left most column is 0;
      buffer_index = getPixelIndex(buffer_column, row);  // The right column of the buffer needs to be added
      matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
    }
  }
}

void shiftMatrixHorizontal(const char * Direction){
  if(!strcmp("Right", Direction)){
    // Shifting columns, not writing the leftmost column here
    size_t oldIndex = 0;
    size_t newIndex = 0;
    for(int col = width - 2; col >= 0; col--){
      for(int row = 0; row < height; row ++){
        oldIndex = getPixelIndex(col,row);
        newIndex = getPixelIndex(col + 1,row);
        matrix.copyIndex(oldIndex, newIndex);
      }
    }
  // Shifting towards the left
  } else {
    size_t oldIndex = 0;
    size_t newIndex = 0;
    for(int col = 0; col < width - 1; col++){
      for(int row = 0; row < height; row++){
        oldIndex = getPixelIndex(col + 1, row);
        newIndex = getPixelIndex(col, row);
        matrix.copyIndex(oldIndex, newIndex);
      }
    }
  }
}
void shiftMatrixVertical(const char * Direction){
  if(!strcmp("Up", Direction)){
    // Shifting row upwards, the bottom most row is reserved for new image, or a wrap around of the first image
    size_t oldIndex = 0;
    size_t newIndex = 0;
    for(int row = height - 2; row >= 0; row--){
      for(int col = 0; col < width; col ++){
        oldIndex = getPixelIndex(col,row);
        newIndex = getPixelIndex(col,row+1);
        matrix.copyIndex(oldIndex, newIndex);
      }
    }
  // Shifting towards the bottom, the top-most row is reserved for new image, or wrap around of the first image
  } else {
    size_t oldIndex = 0;
    size_t newIndex = 0;
    for(int row = 0; row < height - 1; row++){
      for(int col = 0; col < width; col++){
        oldIndex = getPixelIndex(col, row+1);
        newIndex = getPixelIndex(col, row);
        matrix.copyIndex(oldIndex, newIndex);
      }
    }
  }
}
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
void addRow(const char * Direction, uint32_t * buffer, uint32_t buffer_row){
  if(!strcmp("Up", Direction)){ // The frame is shifting up
    size_t matrix_index = 0;
    size_t buffer_index = 0;

    for(int col = 0; col < height; col++){
      matrix_index = getPixelIndex(col, 0);  // The bottom row is matrix_index 0
      buffer_index = getPixelIndex(col, height - 1 - buffer_row);  // The top row of the buffer needs to be added
      matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
    }
  } else { // The frame is shifting down
    size_t matrix_index = 0;
    size_t buffer_index = 0;
    for(int col = 0; col < height; col++){
      matrix_index = getPixelIndex(col, height-1); // The top most row is matrix_index = height - 1
      buffer_index = getPixelIndex(col, buffer_row); // The bottom row from the buffer needs to be added
      matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
    }
  }
}
void SinglePixel(uint8_t rowindex, uint8_t previndex, uint32_t color){
  matrix.setPixelRGB( previndex, 0);
  matrix.setPixelRGB( rowindex, color);
}

// Maps the associated pixel in the matrix to the index of the contiguous array
size_t getPixelIndex(int x, int y){
  if(!strcmp("CJMCU-64", matrixType.c_str())){
    return 8*(x%8)+(y%8) +  64*((int)(x/8)) + width*8*((int)(y/8));
  } else if (!strcmp("WS-2812-8x8", matrixType.c_str())){
      // Add the two options for the extra matrix types
    if(x%2 == 1){
      return 8*(x%8)+(7 - y%8) +  64*((int)(x/8)) + width*8*((int)(y/8));
    } else {
      return 8*(x%8)+(y%8) +  64*((int)(x/8)) + width*8*((int)(y/8));
    }
  } else if (!strcmp("CUSTOM-CJMCU", matrixType.c_str())){
    return y + x * height;
  } else if (!strcmp("CUSTOM-WS-2812", matrixType.c_str())){
    if(x%2 ==1){
      return height - 1 - y + x*height;
    } else {
      return y + x * height;
    }
  } else {
    return 0;
  }
}