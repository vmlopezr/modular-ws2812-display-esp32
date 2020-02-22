#ifndef WEBSERVERTASK_H
#define WEBSERVERTASK_H

#include "Globals.h"
#include "Filesystem.h"

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void updateDisplaySize(const char * data, size_t &height, size_t &width);
void readFileAction(uint8_t num, uint8_t * payload );
void writeFileAction(uint8_t num, size_t length, uint8_t * payload );
void appendFileAction(uint8_t num, size_t length, uint8_t * payload );
void liveInputState( uint8_t * payload);
void clearLiveInput();
void exitLiveInputState();
void receivedLiveInput(uint8_t * payload);
void clearFrameAction();
void AnimationAction(const char * animationLabel);
#endif