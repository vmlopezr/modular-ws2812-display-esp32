
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "WiFi.h"
#include "WebSocketsServer.h"
#include "Globals.h"

// clearBuffer can be re-used for any array
void clearBuffer(uint8_t *array ,uint16_t length);

// The functions below depend on global data from Globals.h.
// These are defined in main.cpp. The functions are used solely for this project.
// They may not have reusability without some tweaking.
const char * listRootDir();
std::string extractFilename(uint8_t * payload);
void readfile(uint8_t &client,uint8_t * filename);
uint16_t readfileSize(File &fileptr);
void writefile(uint8_t client, const char * filename, uint8_t * payload, const char* suffix);
void writefile(const char *filename, const char *payload);
void renamefile(const char *path1, const char * path2);
void deletefile(const char *path);
void appendfile(uint8_t client, const char * filename, uint8_t * payload, const char * suffix);
bool readDefaultFrames(const char * defaultData);
void StartUpDefaultFrame();
void updateFrameData(size_t arraySize);
void resetFrameData();
void resetBuffer(uint32_t * buffer);
#endif
