#ifndef GLOBALS_H
#define GLOBALS_H

#include "WebSocketsServer.h"
#include "Esp32CtrlLed.h"

#define MAX_BUFFER_SIZE 6500

// Globals, defined in main.cp
extern const char* ssid;
extern const char* password;
extern size_t height;
extern size_t width;
extern std::string S_height;
extern std::string S_width;
extern std::string matrixType;
extern bool isCJMCU;
extern uint8_t stateMachine[5];
extern uint8_t filename[100];
extern uint8_t animation[5];
extern std::string *FileNames;
extern std::string *Effects;
extern std::string *DisplayTime;
extern std::string *Direction;
extern std::string *SlideSpeed;
extern size_t numSavedFrames;
extern uint8_t appDataBuffer[MAX_BUFFER_SIZE];
extern uint32_t *LEDBuffer1;
extern uint32_t *LEDBuffer2;
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
extern bool fileLock;
extern int testValue;
extern volatile bool  writeFrameISR;
extern hw_timer_t * timer;
extern portMUX_TYPE timerMux;

//Interrupt Service Routine for the Default State
void IRAM_ATTR onTimer();

#endif