#include "LedEffectsLib.h"

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
void HorizontalLine(uint8_t rowIndex, uint8_t prevRowIndex, size_t width, size_t height, uint32_t color)
{
    if (rowIndex < height)
    {
        int index = 0;
        int prevIndex = 0;
        for (int col = 0; col < width; col++)
        {
            index = getPixelIndex(col, rowIndex);
            prevIndex = getPixelIndex(col, prevRowIndex);
            matrix.setPixelRGB(prevIndex, 0);
            matrix.setPixelRGB(index, color);
        }
    }
}
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
void VerticalLine(uint8_t colIndex, uint8_t prevColIndex, size_t width, size_t height, uint32_t color)
{
    if (colIndex < width)
    {
        int index = 0;
        int prevIndex = 0;
        for (int row = 0; row < height; row++)
        {
            index = getPixelIndex(colIndex, row);
            prevIndex = getPixelIndex(prevColIndex, row);
            matrix.setPixelRGB(prevIndex, 0);
            matrix.setPixelRGB(index, color);
        }
    }
}
/**
  @brief Displays a single led at a time. Iterates from starting index to the max
  size (width * height - 1). This can be used to see how the data signal is routed
  in the display.

  @param newIndex: size_t -> New index to be turned on.
  @param prevIndex: size_t -> Previous index to be cleared on the display.
  @param color: uint32_t -> 24-bit color of the new pixel. formatted as 0x00RRGGBB
*/
void SinglePixel(size_t newIndex, size_t prevIndex, uint32_t color)
{
    matrix.setPixelRGB(prevIndex, 0);
    matrix.setPixelRGB(newIndex, color);
}

/**
  @brief Adds a row from the buffer into the matrix display array. This function
  is used on the vertical sliding effect.

  ex: When the display frame is shifted up one row, the bottom row becomes empty.
  addRow() thus adds the top most row of the next frame stored in the buffer.

  Note: This function is excuted at least height = n times. The bufferRow input is a
  sequence starting from 0 to height-1.
  @param direction The direction at which the frame is sliding.
  @param buffer  The array on which the next frame is stored.
  @param bufferRow The row index of the buffer that is added to the matrix display array.
*/
void addRow(const char *direction, uint32_t *buffer, uint32_t bufferRow)
{
    if (!strcmp("Up", direction))
    {
        size_t matrix_index = 0;
        size_t buffer_index = 0;
        for (int col = 0; col < height; col++)
        {
            // The bottom row of the display matrix is index 0
            matrix_index = getPixelIndex(col, 0);
            // The top row of the buffer needs to be added
            buffer_index = getPixelIndex(col, height - 1 - bufferRow);
            matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
        }
    }
    // The frame is shifting down
    else
    {
        size_t matrix_index = 0;
        size_t buffer_index = 0;
        for (int col = 0; col < height; col++)
        {
            // The top most row is matrix_index = height - 1
            matrix_index = getPixelIndex(col, height - 1);
            // The bottom row from the buffer needs to be added
            buffer_index = getPixelIndex(col, bufferRow);
            matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
        }
    }
}

/**
  @brief Adds a column from the buffer into the matrix display array. This function
  is used on the horizontal sliding effect.

  ex: When the display frame is shifted to the left one row, the right-most column becomes empty.
  addColumn() thus adds the left-most column of the next frame stored in the buffer. The opposite
  is the case when shifting to the right.

  Note: This function is excuted at least width = n times. The bufferColumn input is a
  sequence starting from 0 to height-1.
  @param direction The direction at which the frame is sliding.
  @param buffer  The array on which the next frame is stored.
  @param bufferColumn The row index of the buffer that is added to the matrix display array.
*/
void addColumn(const char *direction, uint32_t *buffer, uint32_t buffer_column)
{
    if (!strcmp("Right", direction))
    {
        size_t matrix_index = 0;
        size_t buffer_index = 0;

        for (int row = 0; row < height; row++)
        {
            // The left-most column in the matrix array is at index 0;
            matrix_index = getPixelIndex(0, row);
            // The right-most column of the buffer needs to be added;
            buffer_index = getPixelIndex(width - 1 - buffer_column, row);
            matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
        }
    }
    // Shifting towards the left
    else
    {
        size_t matrix_index = 0;
        size_t buffer_index = 0;
        for (int row = 0; row < height; row++)
        {
            // The right-most column is at index width - 1
            matrix_index = getPixelIndex(width - 1, row);
            // The left-most column of the buffer needs to be added
            buffer_index = getPixelIndex(buffer_column, row);
            matrix.setPixelRGB(matrix_index, buffer[buffer_index]);
        }
    }
}

/**
  @brief Shifts the currently displayed frame horizontally (right or left) by one column.
  In other words, the matrix display array is shifted. As a result either the left-most (when shifting to the right)
  or the right-most(when shifting to the left) columns in the display are left empty.
  addColumn() is called immediately after this in the state machine to fill the missing column.

  @param direction The direction at which the displayed frame is shifting.
*/
void shiftMatrixHorizontal(const char *direction)
{
    if (!strcmp("Right", direction))
    {

        size_t oldIndex = 0;
        size_t newIndex = 0;
        // Shifting columns, not writing the leftmost column here.
        for (int col = width - 2; col >= 0; col--)
        {
            for (int row = 0; row < height; row++)
            {
                oldIndex = getPixelIndex(col, row);
                newIndex = getPixelIndex(col + 1, row);
                matrix.copyIndex(oldIndex, newIndex);
            }
        }
    }
    // Shifting towards the left
    else
    {
        size_t oldIndex = 0;
        size_t newIndex = 0;
        for (int col = 0; col < width - 1; col++)
        {
            for (int row = 0; row < height; row++)
            {
                oldIndex = getPixelIndex(col + 1, row);
                newIndex = getPixelIndex(col, row);
                matrix.copyIndex(oldIndex, newIndex);
            }
        }
    }
}
/**
  @brief Shifts the currently displayed frame vertically (up or down) by one row.
  In other words, the matrix display array is shifted. As a result either the top-most (when shifting to the bottom)
  or the bottom(when shifting upwards) rows in the display are left empty.
  addRow() is called immediately after this in the state machine to fill the missing row.

  @param direction The direction at which the displayed frame is shifting.
*/
void shiftMatrixVertical(const char *direction)
{
    if (!strcmp("Up", direction))
    {
        // Shifting row upwards, the bottom most row is reserved for new image row,
        // or a wrap around of the first image
        size_t oldIndex = 0;
        size_t newIndex = 0;
        for (int row = height - 2; row >= 0; row--)
        {
            for (int col = 0; col < width; col++)
            {
                oldIndex = getPixelIndex(col, row);
                newIndex = getPixelIndex(col, row + 1);
                matrix.copyIndex(oldIndex, newIndex);
            }
        }
    }
    else
    {
        size_t oldIndex = 0;
        size_t newIndex = 0;
        // Shifting towards the bottom, the top-most row is reserved for new image row,
        // or wrap around of the first image
        for (int row = 0; row < height - 1; row++)
        {
            for (int col = 0; col < width; col++)
            {
                oldIndex = getPixelIndex(col, row + 1);
                newIndex = getPixelIndex(col, row);
                matrix.copyIndex(oldIndex, newIndex);
            }
        }
    }
}

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
size_t getPixelIndex(const size_t x, const size_t y)
{
    if (!strcmp("CJMCU-64", matrixType.c_str()))
    {
        // The data signal on each column moves from bottom to top
        return 8 * (x % 8) + (y % 8) + 64 * ((size_t)(x / 8)) + width * 8 * ((size_t)(y / 8));
    }
    else if (!strcmp("WS-2812-8x8", matrixType.c_str()))
    {
        // The data signal on odd columns moves from top to bottom
        if (x % 2 == 1)
        {
            return 8 * (x % 8) + (7 - y % 8) + 64 * ((size_t)(x / 8)) + width * 8 * ((size_t)(y / 8));
        }
        // The data signal on even columns moves from bottom to top
        else
        {
            return 8 * (x % 8) + (y % 8) + 64 * ((size_t)(x / 8)) + width * 8 * ((size_t)(y / 8));
        }
    }
    else if (!strcmp("CUSTOM-CJMCU", matrixType.c_str()))
    {
        // The data signal on each column moves from bottom to top
        return y + x * height;
    }
    else if (!strcmp("CUSTOM-WS-2812", matrixType.c_str()))
    {
        // The data signal on odd columns moves from top to bottom
        if (x % 2 == 1)
        {
            return height - 1 - y + x * height;
        }
        // The data signal on even columns moves from bottom to top
        else
        {
            return y + x * height;
        }
    }
    else
    {
        return 0;
    }
}