#ifndef WS2812SDCARD_H
#define WE2812SDCARD_H

#include "Esp32CtrlLed.h"
#include "SD.h"
#include "FS.h"
#include "Globals.h"

/**
  @brief Opens a given file and reads the frame data into the matrix display array.
  Note: The first row of all frame files is used to give file information.
   - When reading such files, skip the first row.
   - The led data begins on the second row. Every 6 characters is the string value for the
     pixel color. e.g. "RRGGBB", this will then get converted to a hex unsigned integer.
   - Start populating the matrix display array from index 0 up to its length - 1.

  @param Leds     The LED control object. Contains the matrix display array.
  @param filename The filename that is opened and read.
*/
void loadDataToMatrixDisplay(Esp32CtrlLed &Leds, const char *filename);

/**
  @brief Opens a given file and reads the frame data into the buffer array.
  Note: The first row of all frame files is used to give file information.
   - When reading such files, skip the first row.
   - The led data begins on the second row. Every 6 characters is the string value for the
     pixel color. e.g. "RRGGBB", this will then get converted to a hex unsigned integer.
   - Start populating the matrix display array from index 0 up to its length - 1.

  Global Variables Used:
    - matrix: The LED control object that contains the matrix display array.
  @param buffer   The buffer array; contains the data for the next frame to be displayed.
  @param filename The filename that is opened and read.
*/
void loadDataToBuffer(uint32_t *buffer, const char *filename);

/**
  @brief Write the data of the next frame stored on the buffer array, to the
  matrix display array.

  @param buffer  The buffer array; contains the data for the next frame to be displayed.
*/
void writeBufferToLed(uint32_t *buffer);

#endif