#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "WiFi.h"
#include "WebSocketsServer.h"

#define MAX_BUFFER_SIZE 100

void clearBuffer(uint8_t *buffer, uint8_t length);
const char * listRootDir(fs::FS &fs);
std::string extractFilename(uint8_t * payload);
void readfile(fs::FS &fs, uint8_t *buffer, WebSocketsServer &server, uint8_t &client,uint8_t * payload);
void writefile(fs::FS &fs,uint8_t *buffer, WebSocketsServer &server, uint8_t client, const char * filename, uint8_t * payload);
// void appendfile(fs::FS &fs,uint8_t *buffer, WebSocketsServer &server, uint8_t client, uint8_t * payload);
void renamefile(fs::FS &fs, const char * path1, const char * path2);
void deletefile(fs::FS &fs, const char * path);

#endif
