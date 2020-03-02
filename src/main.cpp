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
size_t height = 1;
size_t width = 1;
size_t FLENGTH = 0;
std::string matrixType = "";
bool isCJMCU = true;

// String data for the Led Driver
uint8_t stateMachine[5];
uint8_t filename[100];
uint8_t animation[4];
std::string *Delays;
std::string *DisplayTime;
std::string *FileNames;
std::string *Effects;

// data buffer
uint8_t appDataBuffer[MAX_BUFFER_SIZE];
uint32_t *LEDBuffer1;
uint32_t *LEDBuffer2;

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



void WebServerTask(void *parameter){


  while(1){
    // Look for and handle WebSocket data
    server.loop();
  }
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

  // Led Driver Setup
  matrix.updateLength(64);
  matrix.setPin(GPIO_NUM_26);
  matrix.ESP32_RMT_Init();

  // Read Default frame data into RAM
  StartUpDefaultFrame();


  /* Code below to be used for access point.
  * Connect to Wi-Fi network with SSID and password
  */
  // Serial.print("Setting AP (Access Point)…");

  /* Remove the password parameter, if you want the AP (Access Point) to be open */
  // WiFi.softAP(ssid, password);
  // IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  // /* The following section is used for connecting to network wifi*/
  // Using wifi connection for phone app testing
  // WiFi.begin("ATT2sca5xw", "3#9jry27c%f4");
  // WiFi.begin("DESKTOP-BDJPBC7 9421", "R0)o9854");

  WiFi.begin("UHWireless","");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // HTTP access points
  Serial.println(WiFi.localIP());

   // Start WebSocket server and assign callback
  server.begin();
  server.onEvent(onWebSocketEvent);

  xTaskCreatePinnedToCore(WebServerTask, "WebTask", 10000, NULL, 1, &Web_Server_Task,1);
  xTaskCreatePinnedToCore(LedDriverTask, "ESPTask", 10000, NULL, 1, &Led_Driver_Task,0);

}


void loop(){}
