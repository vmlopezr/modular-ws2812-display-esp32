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
extern size_t FLENGTH;
extern std::string matrixType;
extern bool isCJMCU;
extern uint8_t stateMachine[5];
extern uint8_t filename[100];
extern uint8_t animation[4];
extern std::string *Delays;
extern std::string *DisplayTime;
extern std::string *FileNames;
extern std::string *Effects;
extern uint8_t appDataBuffer[MAX_BUFFER_SIZE];
extern uint8_t *LEDBuffer1;
extern uint8_t *LEDBuffer2;
extern TaskHandle_t Web_Server_Task;
extern TaskHandle_t Led_Driver_Task;
extern WebSocketsServer server;
extern Esp32CtrlLed matrix;
extern bool bufferLock;
extern bool appInput;
extern bool animationState;
extern bool liveInputState;
extern bool receivedLiveData;
extern bool defaultState;
extern bool bufferLock;
extern int testValue;




#endif