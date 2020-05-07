#ifndef GLOBALS_H
#define GLOBALS_H

#include "WebSocketsServer.h"
#include "Esp32CtrlLed.h"

#define MAX_BUFFER_SIZE 6500

// Globals, all defined in main.cpp

// Wireless Access Point information
extern const char *ssid;
extern const char *password;

// LED display information
extern size_t height;
extern size_t width;
extern std::string S_height;
extern std::string S_width;
extern std::string matrixType;
extern bool isCJMCU;

// Data receive buffers
extern uint8_t stateMachine[5];
extern uint8_t filename[100];
extern uint8_t animation[5];
extern uint8_t appDataBuffer[MAX_BUFFER_SIZE];
extern uint32_t *LEDBuffer1;

// Default Frame list variables
extern std::string *FileNames;
extern std::string *Effects;
extern std::string *DisplayTime;
extern std::string *Direction;
extern std::string *SlideSpeed;
extern std::string *BlinkTime;
extern size_t numSavedFrames;

// FreeRTOS Task handles
extern TaskHandle_t Web_Server_Task;
extern TaskHandle_t Led_Driver_Task;

extern WebSocketsServer server;

// Led Control object
extern Esp32CtrlLed matrix;

//LED Driver flags
extern bool bufferLock;
extern bool appInput;
extern bool animationState;
extern bool liveInputState;
extern bool receivedLiveData;
extern bool defaultState;
extern bool fileLock;

// Timer variables
extern volatile bool writeFrameISR;
extern hw_timer_t *timer;
extern portMUX_TYPE timerMux;

//Interrupt Service Routine for the Default State
void IRAM_ATTR onTimer();

#endif