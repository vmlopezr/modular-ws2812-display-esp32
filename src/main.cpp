#include "WiFi.h"
#include "SD.h"
#include "WebSocketsServer.h"
#include "Filesystem.h"
// #include "LEDMatrix.h"

const char* ssid = "Smart_Billboard_AP";
const char* password =  "12345678";

uint16_t char_available = 0;
uint8_t buffer[MAX_BUFFER_SIZE];
TaskHandle_t Web_Server_Task;
TaskHandle_t Led_Driver_Task;

// Globals
WebSocketsServer webSocket = WebSocketsServer(80);


/*
 * Copyright (c) 2018, circuits4you.com
 * All rights reserved.
   Generates PWM on Internal LED Pin GPIO 2 of ESP32*/

#define LED 4 //On Board LED

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 10; //Resolution 8, 10, 12, 15


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
          webSocket.sendTXT(num,listRootDir(SD, buffer));
          // Serial.printf("finished sending file names\n");

      } else if (!strcmp("read", (const char *)buffer)){
          clearBuffer(buffer,4);
          readfile(SD, buffer, webSocket, num,  (payload+4));
          // Serial.printf("finished reading file\n");

      } else if(!strcmp("save", (const char*)buffer)){
          Serial.printf("Received size: %d\n", length);
          clearBuffer(buffer,4);
          std::string filename = extractFilename(payload+4);
          writefile(SD,buffer, webSocket, num, filename.c_str(), (payload+4+filename.length()));

      } else if(!strcmp("apnd", (const char*)buffer)){
          clearBuffer(buffer,4);
          std::string filename = extractFilename(payload+4);
          appendfile(SD, buffer, webSocket, num, filename.c_str(), (payload+4+filename.length()));
      } else if(!strcmp("EXWT",(const char*)buffer)){
          webSocket.sendTXT(num, "sucessWrite");
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
  //PWM Value varries from 0 to 1023
  Serial.println("10 % PWM");
  ledcWrite(ledChannel, 102);
  delay(2000);

  Serial.println("20 % PWM");
  ledcWrite(ledChannel,205);
  delay(2000);

  Serial.println("40 % PWM");
  ledcWrite(ledChannel,410);
  delay(2000);

  Serial.println("70 % PWM");
  ledcWrite(ledChannel,714);
  delay(2000);

  Serial.println("100 % PWM");
  ledcWrite(ledChannel,1024);
  delay(2000);

  //Continuous Fading
  Serial.println("Fadding Started");
  while(1)
  {
    // set the brightness of pin 2:
    ledcWrite(ledChannel, brightness);

    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;

    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 1023) {
      fadeAmount = -fadeAmount;
    }
    // wait for 30 milliseconds to see the dimming effect
    delay(10);
  }
}



//=======================================================================
//                    Power on setup
//=======================================================================
void setup(){
  clearBuffer(buffer, MAX_BUFFER_SIZE);
  // Set Serial baud rate
  Serial.begin(9600);

  // Initialize SD card
  if( !SD.begin()){
    Serial.println("SD Card could not be initialized.");
  }

  pinMode(LED,OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);

  // attach the channel to the GPIO2 to be controlled
  ledcAttachPin(LED, ledChannel);


  /* Code below to be used for access point.
  * Connect to Wi-Fi network with SSID and password
  */
  Serial.print("Setting AP (Access Point)…");

  /* Remove the password parameter, if you want the AP (Access Point) to be open */
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  /* The following section is used for connecting to network wifi*/
  // Using wifi connection for phone app testing
  // WiFi.begin("ATT2sca5xw", "3#9jry27c%f4");

  // WiFi.begin("UHWireless","");

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi..");
  // }

  // // HTTP access points
  // Serial.println(WiFi.localIP());

   // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  xTaskCreatePinnedToCore(WebServerTask, "WebTask", 10000, NULL, 1, &Web_Server_Task,1);
  xTaskCreatePinnedToCore(LedDriverTask, "ESPTask", 10000, NULL, 1, &Led_Driver_Task,0);

}


void loop(){}
