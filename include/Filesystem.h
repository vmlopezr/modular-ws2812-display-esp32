
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "WiFi.h"
#include "WebSocketsServer.h"
#include "Globals.h"

/**
  @brief Clear a contiguous buffer by refilling with the null character.

  @param buffer Array to be cleared
  @param length Number of entries  to clear
*/
void clearBuffer(uint8_t *buffer, uint16_t length);

/** The functions below depend on global data from Globals.h.
 *  These are defined in main.cpp. The functions are used solely for this project.
 *  They may not have reusability without some tweaking.
*/
/**
  @brief  Retrieve a file list of root directory.
  @return An array of all files in the root directory of the SD card
*/
const char *listRootDir();

/**
  @brief Extract a filename from a data buffer. Looks for the first '.' in the
         buffer.
  @param payload Data buffer received from Websocket Server
  @return Extracted filename, contains up to three characters after the '.' found.
          Default return: "default.txt"  if no '.' found in buffer.
*/
std::string extractFilename(uint8_t *payload);

/**
  @brief Read file and and send data to client
  global vars used: WebSocketsServer server

  @param client code corresponding to client
  @param filename file to read
*/
void readfile(const uint8_t client, uint8_t *filename);

/**
  @brief The first line of each file contains the amount of characters in the file.
         Read the first line and return it.
  @param fileptr Pointer to the given file.
  @return File size, in characters, for the given file pointer.
*/
uint16_t readfileSize(File &fileptr);

/**
  @brief Write payload data to a file in the SD card root directory

  @param client Code number for the client.
  @param filename Name of the file with no file extension
  @param payload Data received from client
  @param eofCode End of file code.
*/
void writefile(uint8_t client, const char *filename, uint8_t *payload, const char *eofCode);

/**
  @brief Write payload data to a file in the SD card root directory

  @param filename Name of the file with no file extension
  @param payload Data received from client
*/
void writefile(const char *filename, const char *payload);

/**
  @brief Rename a file.

  @param path1 File to be renamed
  @param path2 New name for the file.
*/
void renamefile(const char *path1, const char *path2);

/**
  @brief Delete the file in the SD card.

  @param path File to be deleted
*/
void deletefile(const char *path);

/**
  @brief Append data to existing file.

  @param client Code number for the client.
  @param filename Name of the file to be written.
  @param payload Data received from the client.
  @param eofCode End of file code
*/
void appendfile(uint8_t client, const char *filename, uint8_t *payload, const char *eofCode);

/**
  @brief Loads global variables with previous user data as well as
         the frame sequence to be displayed by default.

  Global Variables:
    - numSavedFrames size_t     -> Number of frames in the display loop.
    - S_height: std::string     -> Height of LED display.
    - height: size_t            -> Height of LED display.
    - S_width: std::string      -> Width of LED display.
    - width: size_t             -> Width of LED display.
    - matrixType: std::string   -> Type of matrix chosen by the user
    - Filenames: std:string*    -> Array containing the filenames to read.
    - Effects: std::string*     -> Array of Effects for each corresponding frame.
    - DisplayTime: std::string* -> Total number of time a frame is displayed.
    - Direction: std::string*   -> Sliding direction for the slide effect.
    - SlideSpeed: std::string*  -> Amount of time before shifting frame along column or row.
    - BlinkTime: std::string*   -> Amount of time a frame is on, and off; e.g. 1 second

  @param defaultData The content of the "DefaultDisplay.txt" file.
  @return Boolean denoting the success of the read. "false" if errors occur, "true" otherwise.
*/
bool readDefaultFrames(const char *defaultData);

/**
  @brief Read "DefaultDisplay.txt" in the Production folder. On successful read
         it sends the data to readDefaultFrames to load data to global variables.

         Loads global variables with default values in the event of incorrect "DefaultDisplay.txt"
         data format.
*/
void StartUpDefaultFrame();

/**
  @brief Resize the dynamic global arrays. Function is used when user changes display
  size on the phone application.

  @param arraySize The new size of the default display std::string* arrays.
*/
void updateFrameData(size_t arraySize);

/**
  @brief Clear the global std::string* arrays for default display loop.
*/
void resetFrameData();

/**
  @brief Clear the LED display buffer.

  @param buffer The array to clear.

*/
void resetBuffer(uint32_t *buffer);

/**
  @brief Convert a unsigned integer to a string.
  @param number Input number
  @return String version of the input number. 34 -> "34"
*/
std::string to_string(unsigned int number);
#endif
