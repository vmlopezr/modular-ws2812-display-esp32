
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "WiFi.h"
#include "WebSocketsServer.h"

#define MAX_BUFFER_SIZE 6500
void clearBuffer(uint8_t *buffer, uint16_t length);
const char * listRootDir(fs::FS &fs, uint8_t * buffer);
std::string extractFilename(uint8_t * payload);
void readfile(fs::FS &fs, uint8_t *buffer, WebSocketsServer &server, uint8_t &client,uint8_t * filename);
uint16_t readfileSize(fs::FS &fs, uint8_t * buffer, File &fileptr);
void writefile(fs::FS &fs,uint8_t *buffer, WebSocketsServer &server, uint8_t client, const char * filename, uint8_t * payload, const char* suffix);
void renamefile(fs::FS &fs, const char * path1, const char * path2);
void deletefile(fs::FS &fs, const char * path);
void appendfile(fs::FS &fs, uint8_t *buffer, WebSocketsServer &server, uint8_t client, const char * filename, uint8_t * payload, const char * suffix);
#endif
