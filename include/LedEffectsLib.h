#ifndef LEDEFFECTSLIB_H
#define LEDEFFECTSLIB_H

#include "Esp32CtrlLed.h"
#include "Globals.h"

/**
  @brief Displays a horizontal line at the row Index, and clears the previously drawn row.
  The row index starts from 0 to height - 1

  Global Variables Used:
    - matrix: Esp32CtrlLed      -> Object that loads saved data to the LED display.
  @param rowIndex: uint8_t      -> The new row index. The row to be displayed.
  @param prevRowIndex: uint8_t  -> The previous row index. This row will be cleared,
  @param width: size_t          -> The width (in pixels) of the LED display
  @param height: size_t         -> The height (in pixels) of the LED display
  @param color: uint32_t        -> 24-bit color of the new pixel. formatted as 0x00RRGGBB
*/
void HorizontalLine(uint8_t rowIndex, uint8_t prevRowIndex, size_t width, size_t height, uint32_t color);

/**
  @brief Displays a vertical line at the column Index, and clears the previously drawn column.
  The column index starts from 0 to width - 1

  Global Variables Used:
    - matrix: Esp32CtrlLed      -> Object that loads saved data to the LED display.
  @param colIndex: uint8_t      -> The new column index. The column to be displayed.
  @param prevColIndex: uint8_t  -> The previous column index. This column will be cleared,
  @param width: size_t          -> The width (in pixels) of the LED display
  @param height: size_t         -> The height (in pixels) of the LED display
  @param color: uint32_t        -> 24-bit color of the new pixel. formatted as 0x00RRGGBB
*/
void VerticalLine(uint8_t colIndex, uint8_t prevColIndex, size_t width, size_t height, uint32_t color);

/**
  @brief Displays a single led at a time. Iterates from starting index to the max
  size (width * height - 1). This can be used to see how the data signal is routed
  in the display.
  Global Variables Used:
    - matrix: Esp32CtrlLed -> Object that loads saved data to the LED display.
  @param newIndex: size_t  -> New index to be turned on.
  @param prevIndex: size_t -> Previous index to be cleared on the display.
  @param color: uint32_t   -> 24-bit color of the new pixel. formatted as 0x00RRGGBB
*/
void SinglePixel(size_t newIndex, size_t prevIndex, uint32_t color);

/**
  @brief Adds a row from the buffer into the matrix display array. This function
  is used on the vertical sliding effect.

  ex: When the display frame is shifted up one row, the bottom row becomes empty.
  addRow() thus adds the top most row of the next frame stored in the buffer.

  Note: This function is excuted at least height = n times. The bufferRow input is a
  sequence starting from 0 to height-1.

  Global Variables Used:
    - matrix: Esp32CtrlLed      -> Object that loads saved data to the LED display.
  @param direction The direction at which the frame is sliding.
  @param buffer  The array on which the next frame is stored.
  @param bufferRow The row index of the buffer that is added to the matrix display array.
*/
void addRow(const char *direction, uint32_t *buffer, uint32_t bufferRow);

/**
  @brief Adds a column from the buffer into the matrix display array. This function
  is used on the horizontal sliding effect.

  ex: When the display frame is shifted to the left one row, the right-most column becomes empty.
  addColumn() thus adds the left-most column of the next frame stored in the buffer. The opposite
  is the case when shifting to the right.

  Note: This function is excuted at least width = n times. The bufferColumn input is a
  sequence starting from 0 to height-1.
  Global Variables Used:
    - matrix: Esp32CtrlLed      -> Object that loads saved data to the LED display.
  @param direction The direction at which the frame is sliding.
  @param buffer  The array on which the next frame is stored.
  @param bufferColumn The row index of the buffer that is added to the matrix display array.
*/
void addColumn(const char *direction, uint32_t *buffer, uint32_t bufferColumn);

/**
  @brief Shifts the currently displayed frame horizontally (right or left) by one column.
  In other words, the matrix display array is shifted. As a result either the left-most (when shifting to the right)
  or the right-most(when shifting to the left) columns in the display are left empty.
  addColumn() is called immediately after this in the state machine to fill the missing column.
  Global Variables Used:
    - matrix: Esp32CtrlLed      -> Object that loads saved data to the LED display.
  @param direction The direction at which the displayed frame is shifting.
*/
void shiftMatrixHorizontal(const char *direction);

/**
  @brief Shifts the currently displayed frame vertically (up or down) by one row.
  In other words, the matrix display array is shifted. As a result either the top-most (when shifting to the bottom)
  or the bottom(when shifting upwards) rows in the display are left empty.
  addRow() is called immediately after this in the state machine to fill the missing row.
  Global Variables Used:
    - matrix: Esp32CtrlLed      -> Object that loads saved data to the LED display.
  @param direction The direction at which the displayed frame is shifting.
*/
void shiftMatrixVertical(const char *direction);

/**
  @brief Uses the (x,y) coordinate of a pixel in the matrix display to calculate the index
  of the corresponding pixel in the matrix pixel array.

  Note: The matrix pixel array is a contiguous 1-dimensional array. The data signal path of the display
  thus dictates how the pixel index must be calculated. Depending on the matrix type a display can be
  composed of stacked 8x8 matrices. Please see the settings help menu in phone application, or the following
  page to get more information about the matrix types supported.

  @param x: The horizontal coordinate (along the width) of a pixel in the LED matrix display.
  @param y: The vertical coordinate (along the height) of a pixel in the LED matrix display.
  @return The index of the pixel at the (x,y) coordinates in the LED matrix display.
*/
size_t getPixelIndex(const size_t x, const size_t y);
#endif