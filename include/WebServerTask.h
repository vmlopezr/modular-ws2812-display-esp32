#ifndef WEBSERVERTASK_H
#define WEBSERVERTASK_H

#include "Globals.h"
#include "Filesystem.h"


void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
bool updateDisplaySize(char * data, size_t &oldHeight, size_t &oldWidth);
void readFileAction(uint8_t num, uint8_t * payload );
void defaultReadAction(uint8_t num,uint8_t * payload);
void writeFileAction(uint8_t num, size_t length, uint8_t * payload );
void appendFileAction(uint8_t num, size_t length, uint8_t * payload );
void liveInputAction( uint8_t * payload);
void clearLiveInput();
void exitLiveInputState();
void receivedLiveInput(uint8_t * payload);
void clearFrameAction();
void updateMatrixTypeData( uint8_t * mType);
void AnimationAction(const char * animationLabel);
void getDefaultFrames(uint8_t client);
void updateBufferLength(int newWidth, int newHeight);
#endif