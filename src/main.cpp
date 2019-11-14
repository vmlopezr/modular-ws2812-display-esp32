#include "WiFi.h"
#include "ESPAsyncWebServer.h"
 
const char* ssid = "Smart_Billboard_AP";
const char* password =  "12345678";
AsyncWebServer server(80);
uint16_t i = 0;
uint16_t j = 0;
TaskHandle_t Task1;
TaskHandle_t Task2; 

void task1(void *parameter){
  // // Code below to be used for access point.
  // // Connect to Wi-Fi network with SSID and password
  // Serial.print("Setting AP (Access Point)…");
  // // Remove the password parameter, if you want the AP (Access Point) to be open
  // WiFi.softAP(ssid, password);

  // IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  // Using wifi connection for phone app testing
  // WiFi.begin("ATT2sca5xw", "3#9jry27c%f4");
  WiFi.begin("UHWireless","");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // HTTP access points
  Serial.println(WiFi.localIP());
  Serial.println(xPortGetCoreID());
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });
  server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"</head><body><h1>Hello, World!</h1></body></html>");
  });
  server.on("/getSize", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, [](AsyncWebServerRequest *request,uint8_t *data, size_t len, size_t index, size_t total){
    for (size_t i = 0; i < len; i++){
      Serial.write(data[i]);
    }
    request->send(200);
  });
  server.begin();
  while(1){}
}
void task2(void *parameter){
  while(1){
    Serial.printf("count j : %d core:%d\n\r",j,xPortGetCoreID());
    j=j+1;
    delay(3000);
  }
}
void setup(){
  Serial.begin(115200);

  xTaskCreatePinnedToCore(task1, "Task1", 10000, NULL, 1, &Task1,1);
  // xTaskCreatePinnedToCore(task2, "Task2", 10000, NULL, 1, &Task2,0);

}

 
void loop(){}