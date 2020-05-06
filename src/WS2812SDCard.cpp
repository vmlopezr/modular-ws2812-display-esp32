#include "WS2812SDCard.h"

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
void loadDataToMatrixDisplay(Esp32CtrlLed &Leds, const char *filename)
{
    File file = SD.open((const char *)filename);
    if (!file)
    {
        Serial.printf("Failed to  open file for reading\n");
        return;
    }
    char temp;

    // Move file pointer to the second line.
    do
    {
        temp = file.read();
        if (temp == '\n' || temp == '\r')
        {
            break;
        }
    } while (file.available());

    int char_count = 0;
    char data[6];
    size_t index = 0;
    size_t color;

    // Read the rest of the data and set the matrix display array
    while (file.available())
    {

        if (char_count >= 6)
        {
            // convert the data array into a 32-bit unsigned integer
            char_count = 0;
            color = strtol((const char *)data, NULL, 16);
            Leds.setPixelRGB(index, color);
            index++;
        }
        temp = file.read();
        // Extract every 6 numeric characters
        if (temp != '\n' && temp != '\r' && temp != ' ')
        {
            data[char_count] = temp;
            char_count++;
        }
    }
    file.close();
}
/**
  @brief Opens a given file and reads the frame data into the buffer array.
  Note: The first row of all frame files is used to give file information.
   - When reading such files, skip the first row.
   - The led data begins on the second row. Every 6 characters is the string value for the
     pixel color. e.g. "RRGGBB", this will then get converted to a hex unsigned integer.
   - Start populating the matrix display array from index 0 up to its length - 1.

  @param buffer   The buffer array; contains the data for the next frame to be displayed.
  @param filename The filename that is opened and read.
*/
void loadDataToBuffer(uint32_t *buffer, const char *filename)
{
    File file = SD.open((const char *)filename);
    if (!file)
    {
        Serial.printf("Failed to  open file for reading\n");
        return;
    }
    char temp;

    // Move file pointer to the second line.
    do
    {
        temp = file.read();
        if (temp == '\n' || temp == '\r')
        {
            break;
        }
    } while (file.available());

    int char_count = 0;
    char data[6];
    size_t index = 0;

    // Read the rest of the data and load it into the buffer array
    while (file.available())
    {
        if (char_count >= 6)
        {
            // convert the data array into a 32-bit unsigned integer
            char_count = 0;
            if (index <= matrix.NUM_LEDS)
            {
                buffer[index] = strtol((const char *)data, NULL, 16);
            }
            index++;
        }
        temp = file.read();
        // Extract every 6 numeric characters
        if (temp != '\n' && temp != '\r' && temp != ' ')
        {
            data[char_count] = temp;
            char_count++;
        }
    }
    file.close();
}
/**
  @brief Write the data of the next frame stored on the buffer array, to the
  matrix display array. Copies the buffer into the display array.

  @param buffer  The buffer array; contains the data for the next frame to be displayed.
*/
void writeBufferToLed(uint32_t *buffer)
{
    for (int i = 0; i < matrix.NUM_LEDS; i++)
    {
        matrix.setPixelRGB(i, LEDBuffer1[i]);
    }
}