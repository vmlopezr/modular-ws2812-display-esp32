#include "Globals.h"
#include "LedDriverTask.h"
#include "WebServerTask.h"
#include "WiFi.h"
#include "SD.h"
#include "WebSocketsServer.h"
#include "Filesystem.h"
#include "Esp32CtrlLed.h"
#include "LedEffectsLib.h"
#include "WS2812SDCard.h"


// Global Definitions
// Access Point Name and Password
const char* ssid = "Smart_Billboard_AP";
const char* password =  "12345678";
// Led Display size dimensions (in # leds)
size_t height = 8;
size_t width = 8;
std::string S_height = "8";
std::string S_width = "8";
std::string matrixType = "";
bool isCJMCU = true;

// String data for the Led Driver
uint8_t stateMachine[5];
uint8_t filename[100];
uint8_t animation[5];
std::string *FileNames = NULL;
std::string *Effects = NULL;
std::string *DisplayTime = NULL;
std::string *Direction = NULL;
std::string *SlideSpeed = NULL;
std::string *BlinkTime = NULL;
size_t numSavedFrames = 0;
// data buffer
uint8_t appDataBuffer[MAX_BUFFER_SIZE];
uint32_t *LEDBuffer1 = NULL;

// Globals for the  Web Socket Task (CPU 1)
TaskHandle_t Web_Server_Task;
TaskHandle_t Led_Driver_Task;
WebSocketsServer server = WebSocketsServer(80);
// Led Object for updating display
Esp32CtrlLed matrix;

// Flags for Led Driver State Machine
//     These are updated by the WebSocket Task (CPU 1) based on App input
bool bufferLock = false;
bool fileLock = false;
bool appInput = false;
bool animationState = false;
bool liveInputState = false;
bool defaultState = false;
bool receivedLiveData = false;

//The timer ISR will be setting this bit to begin frame display.
volatile bool writeFrameISR = false;

//Timer for default display
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


void WebServerTask(void *parameter){


  while(1){
    // Look for and handle WebSocket data
    server.loop();
  }
}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  writeFrameISR = true;
  portEXIT_CRITICAL_ISR(&timerMux);

}
//=======================================================================
//                    Power on setup
//=======================================================================
void setup(){

  // Set Serial baud rate
  Serial.begin(9600);

  // Initialize SD card
  if( !SD.begin()){
    Serial.println("SD Card could not be initialized.");
  }
  // Clear SD card data appDataBuffer
  clearBuffer(appDataBuffer, MAX_BUFFER_SIZE);
  clearBuffer(stateMachine, 5);
  clearBuffer(filename, 100);



  clearBuffer(stateMachine,5);
  clearBuffer(filename,100);
  clearBuffer(animation,5);
  // Read Default frame data into RAM
  // When the file is empty or doesn't exist. Initialize with dummy variables
  StartUpDefaultFrame();
  updateBufferLength(width, height);
  Serial.printf("Size initialized, width: %d, height:%d\n", width,height);
  // Led Driver Setup
  matrix.updateLength(width * height);
  matrix.setPin(GPIO_NUM_26);
  matrix.ESP32_RMT_Init();
  matrix.resetLeds();
  matrix.write_leds();

  /* Code below to be used for access point.
  * Connect to Wi-Fi network with SSID and password
  */
  Serial.print("Setting AP (Access Point)…");

  /* Remove the password parameter, if you want the AP (Access Point) to be open */
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // /* The following section is used for connecting to network wifi*/
  // Using wifi connection for phone app testing
  // WiFi.begin("ATT2sca5xw", "3#9jry27c%f4");
  // WiFi.begin("DESKTOP-BDJPBC7 9421", "R0)o9854");
  // WiFi.begin("UHWireless","");

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi..");
  // }

  // HTTP access points
  // Serial.println(WiFi.localIP());

   // Start WebSocket server and assign callback
  server.begin();
  server.onEvent(onWebSocketEvent);

  // prescaler 80 -> 1MHz counter for timer
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);


  xTaskCreatePinnedToCore(LedDriverTask, "ESPTask", 10000, NULL, 1, &Led_Driver_Task,0);
  xTaskCreatePinnedToCore(WebServerTask, "WebTask", 10000, NULL, 1, &Web_Server_Task,1);


}


void loop(){}
