#ifndef GLOBALS_H
#define GLOBALS_H

#include "WebSocketsServer.h"
#include "Esp32CtrlLed.h"

#define MAX_BUFFER_SIZE 6500

// Globals
extern const char* ssid;
extern const char* password;
extern size_t height;
extern size_t width;
extern uint8_t stateMachine[5];
extern uint8_t filename[100];
extern uint8_t appDataBuffer[MAX_BUFFER_SIZE];
extern TaskHandle_t Web_Server_Task;
extern TaskHandle_t Led_Driver_Task;
extern WebSocketsServer server;
extern Esp32CtrlLed matrix;
extern bool appInput;
extern bool AnimationRunning;
extern bool listenLiveInput;
extern bool defaultState;
extern int testValue;




#endif