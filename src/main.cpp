#include "WiFi.h"
#include "SD.h"
#include "WebSocketsServer.h"
#include "Filesystem.h"

const char* ssid = "Smart_Billboard_AP";
const char* password =  "12345678";

uint16_t char_available = 0;
uint8_t buffer[MAX_BUFFER_SIZE];
TaskHandle_t Web_Server_Task;
TaskHandle_t Led_Driver_Task;

// Globals
WebSocketsServer webSocket = WebSocketsServer(80);

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
    case WStype_TEXT:
      // Extract the first four characters of message
      strncpy((char *)buffer,(const char *)payload, 4);
      if(!strcmp("dirs",(const char*)buffer)){
        webSocket.sendTXT(num,listRootDir(SD));
      } else if (!strcmp("read", (const char *)buffer)){
        clearBuffer(buffer,4);
        readfile(SD, buffer, webSocket, num,  (payload+4));
        Serial.printf("finished reading option\n");
      } else if(!strcmp("save", (const char*)buffer)){
        clearBuffer(buffer,4);
        std::string filename = extractFilename(payload+4);
        writefile(SD,buffer, webSocket, num, filename.c_str(), (payload+4+filename.length()));
      } else if(!strcmp("appd", (const char*)buffer)){
          clearBuffer(buffer,4);
          //use this to append to file.
      }else if(!strcmp("dels", (const char*)buffer)){
        clearBuffer(buffer, 4);
        deletefile(SD, (const char*)(payload+4));
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
  int j = 0;
  while(1){
    Serial.printf("count j : %d core:%d\n\r",j,xPortGetCoreID());
    j=j+1;
    delay(10000);
  }
}
void setup(){
  clearBuffer(buffer, MAX_BUFFER_SIZE);
  // Set Serial baud rate
  Serial.begin(9600);

  // Initialize SD card
  if( !SD.begin()){
    Serial.println("SD Card could not be initialized.");
  }

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
  WiFi.begin("ATT2sca5xw", "3#9jry27c%f4");

  // WiFi.begin("UHWireless","");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // HTTP access points
  Serial.println(WiFi.localIP());

   // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  xTaskCreatePinnedToCore(WebServerTask, "WebTask", 10000, NULL, 1, &Web_Server_Task,1);
  // xTaskCreatePinnedToCore(LedDriverTask, "ESPTask", 10000, NULL, 1, &Led_Driver_Task,0);

}


void loop(){}
