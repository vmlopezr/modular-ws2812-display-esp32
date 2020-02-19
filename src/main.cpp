#include "WiFi.h"
#include "SD.h"
#include "WebSocketsServer.h"
#include "Filesystem.h"
#include "Esp32CtrlLed.h"
#include "LedEffectsLib.h"
#include "WS2812SDCard.h"
#include "Globals.h"

// Globals
const char* ssid = "Smart_Billboard_AP";
const char* password =  "12345678";
size_t height = 1;
size_t width = 1;
uint8_t stateMachine[5];
uint8_t filename[100];
uint8_t buffer[MAX_BUFFER_SIZE];
TaskHandle_t Web_Server_Task;
TaskHandle_t Led_Driver_Task;
WebSocketsServer webSocket = WebSocketsServer(80);
Esp32CtrlLed matrix;
bool appInput = false;
bool AnimationRunning = false;
bool listenLiveInput = false;
bool defaultState = false;
int testValue = 0;

void updateDisplaySize(const char * data, size_t &height, size_t &width){
  uint16_t index = 0;
  uint32_t length = strlen(data);
  std::string size;

  while(index < length){
    if(data[index] != ' '){
      size.push_back(data[index]);
    } else {
      height = strtol(size.c_str(), NULL, 10);
      size.clear();
    }
    index++;
  }
  width = strtol(size.c_str(), NULL, 10);
}
// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {
  clearBuffer(buffer, MAX_BUFFER_SIZE);
  // Figure out the type of WebSocket event
  switch(type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      clearBuffer(stateMachine, 4);
      // strncpy((char *)stateMachine, "DEFT", 4);
      // defaultState = true;
      if(AnimationRunning){
        AnimationRunning = false;
      }
      if(defaultState){
        defaultState = false;
      }
      appInput = false;
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;

    // Echo text message back to client
    // Research using a dictionary like structure to minimize the else if chain.
    case WStype_TEXT:
        // Extract the first four characters of message
        strncpy((char *)buffer,(const char *)payload, 4);
      if(!strcmp("dirs",(const char*)buffer)){
        webSocket.sendTXT(num,listRootDir(SD, buffer));

      // Opens file: 1. Sends data to app.  2. Display data to LED display
      } else if (!strcmp("read", (const char *)buffer)){
        if (defaultState || AnimationRunning || listenLiveInput){
          defaultState = false;
          AnimationRunning = false;
          listenLiveInput = false;
        }
        clearBuffer(buffer,4);
        readfile(SD, buffer, webSocket, num,  (payload+4));

        strcpy((char *)filename, (const char *)(payload+4));
        strncpy((char *)stateMachine, "OPEN", 4);

        appInput = true;


      } else if (!strcmp("size",(const char *)buffer)){
        updateDisplaySize((const char *)(payload+4), height, width);
        Serial.printf("New Display Size\n Height: %d, Width: %d\n", height, width);
      }
      else if(!strcmp("save", (const char*)buffer)){
        strncpy((char *) buffer, (const char *)(payload + length - 4), 4);
        if(!strcmp("EX1T", (const char *)buffer)){
          *(payload+length-4) = '\0';
        }
        std::string suffix = (const char *)buffer;
        std::string filename = extractFilename(payload+4);
        clearBuffer(buffer,4);
        writefile(SD,buffer, webSocket, num, filename.c_str(), (payload+4+filename.length()), suffix.c_str());

      } else if(!strcmp("apnd", (const char*)buffer)){
        strncpy((char *) buffer, (const char *)(payload + length - 4), 4);
        if(!strcmp("EX1T", (const char *)buffer)){
          *(payload+length-4) = '\0';
        }
        std::string suffix = (const char *)buffer;
        std::string filename = extractFilename(payload+4);
        clearBuffer(buffer,4);
        appendfile(SD, buffer, webSocket, num, filename.c_str(), (payload+4+filename.length()), suffix.c_str());

      } else if(!strcmp("EXWT",(const char*)buffer)){
        webSocket.sendTXT(num, "SUXS");

      } else if(!strcmp("dels", (const char*)buffer)){
        clearBuffer(buffer, 4);
        deletefile(SD, (const char*)(payload+4));

      } else if(!strcmp("LINE",(const char*)buffer)){

        strncpy((char *)buffer,(const char *)payload, 4);
        if(listenLiveInput){
          listenLiveInput = false;
        }
        appInput = true;

      } else if(!strcmp("STLI", (const char *)buffer)){
        listenLiveInput = false;
        // Clear the display
        if (defaultState){
          defaultState = false;
        }
        if (AnimationRunning){
          AnimationRunning = false;
        }
        strncpy((char *) stateMachine, "CLCR", 4);
        appInput = true;
      } else if(!strcmp("ANIM", (const char *)buffer)){
        strncpy((char *)stateMachine, "ANIM", 4);

        if (defaultState){
          defaultState = false;
        }
        AnimationRunning = true;
        appInput = true;
      } else {
          webSocket.sendTXT(num, payload);
      }

      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void WebServerTask(void *parameter){
  while(1){
    // Look for and handle WebSocket data
    webSocket.loop();
  }
}

/*
*  task2 to be used for the led driver code
*/
void LedDriverTask(void *parameter){

  // matrix.resetLeds();
  // matrix.write_leds();


  Serial.printf("Led Driver State Machine\n");


  // int newindex = 0;
  // int prev = newindex;

  while(1) {
    if(appInput){

      // Default state - will be used for displaying frames
      if(!strcmp("DEFT", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);
        Serial.printf("displaying default frame\n");
        while(defaultState){


          delay(100);
        }



      } else if(!strcmp("ANIM", (const char*)stateMachine)){
        Serial.printf("Start Animation\n");
        clearBuffer(stateMachine, 4);
        Serial.printf("displaying animated frame\n");
        testValue++;
        Serial.printf("value in main: %d\n", testValue);
        test();
        while(AnimationRunning){
          // if(newindex > 8){
          //   newindex = 0;
          //   prev=7;
          // }
          // HorizontalLine(matrix, newindex, prev, 16, 8, 0x00010000);

          // matrix.write_leds();

          // prev = newindex;
          // newindex++;
          // delay(100);

          // matrix.write_leds();

          delay(100);
        }


        // The open state will display the frame. Then change the statemachine to a "listen for live input" state
      } else if(!strcmp("OPEN", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);

        delay(50);
        // loadDataFromStorage(SD, matrix, (const char *)filename, height, width);
        // matrix.write_leds();

        Serial.printf("filename: %s, height: %d, width: %d\n",(const char *)filename, height, width);

        clearBuffer(filename,strlen((const char *)filename));
        strncpy((char *) stateMachine, "LIVE", 4);
        listenLiveInput = true;

      } else if(!strcmp("CLCR",(const char *) stateMachine)){
        clearBuffer(stateMachine, 4);
        test1();
        Serial.printf("value in main: %d\n",testValue);
        // matrix.resetLeds();
        // matrix.write_leds();
        Serial.printf("Clearing display\n");
        appInput = false;


        // Listen for live input and then display it in the frame
      } else if(!strcmp("LIVE", (const char *)stateMachine)){
        clearBuffer(stateMachine, 4);
        Serial.printf("listening for live input\n");
        while(listenLiveInput){
          delay(100);

        }
        // appInput = false;
      }

    }
    delay(100);

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
  // Clear SD card data buffer
  clearBuffer(buffer, MAX_BUFFER_SIZE);
  clearBuffer(stateMachine, 5);
  clearBuffer(filename, 100);

  // Led Driver Setup
  matrix.updateLength(128);
  matrix.setPin(GPIO_NUM_26);
  matrix.ESP32_RMT_Init();


  /* Code below to be used for access point.
  * Connect to Wi-Fi network with SSID and password
  */
  // Serial.print("Setting AP (Access Point)…");

  /* Remove the password parameter, if you want the AP (Access Point) to be open */
  // WiFi.softAP(ssid, password);
  // IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  /* The following section is used for connecting to network wifi*/
  // Using wifi connection for phone app testing
  // WiFi.begin("ATT2sca5xw", "3#9jry27c%f4");

  WiFi.begin("UHWireless","");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // // HTTP access points
  Serial.println(WiFi.localIP());

   // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  xTaskCreatePinnedToCore(WebServerTask, "WebTask", 10000, NULL, 1, &Web_Server_Task,1);
  xTaskCreatePinnedToCore(LedDriverTask, "ESPTask", 10000, NULL, 1, &Led_Driver_Task,0);

}


void loop(){}
