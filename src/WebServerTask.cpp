#include "WebServerTask.h"
// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  clearBuffer(appDataBuffer, MAX_BUFFER_SIZE);
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
        IPAddress ip = server.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;

    // Echo text message back to client
    // Research using a dictionary like structure to minimize the else if chain.
    case WStype_TEXT:
      // Extract the first four characters of message
      strncpy((char *)appDataBuffer,(const char *)payload, 4);

      if(!strcmp("dirs",(const char*)appDataBuffer)){

        server.sendTXT(num,listRootDir());

      // Opens file: 1. Sends data to app.  2. Display data to LED display
      } else if (!strcmp("read", (const char *)appDataBuffer)){

        readFileAction(num, payload);

      } else if (!strcmp("size",(const char *)appDataBuffer)){

        updateDisplaySize((const char *)(payload+4), height, width);
        matrix.updateLength(height*width);
        // Serial.printf("New Display Size\n Height: %d, Width: %d\n", height, width);
      }
      else if(!strcmp("save", (const char*)appDataBuffer)){

        writeFileAction(num, length, payload);

      } else if(!strcmp("apnd", (const char*)appDataBuffer)){

        appendFileAction(num, length, payload);

      } else if(!strcmp("EXWT",(const char*)appDataBuffer)){

        server.sendTXT(num, "SUXS");

      } else if(!strcmp("dels", (const char*)appDataBuffer)){

        deletefile((const char*)(payload+4));

      } else if(!strcmp("LIVE",(const char*)appDataBuffer)){

        liveInputState(payload);

      } else if(!strcmp("CLRI", (const char*)appDataBuffer)){
        clearLiveInput();
      }else if(!strcmp("EXLI", (const char*)appDataBuffer)){
        exitLiveInputState();
      }else if(!strcmp("INPT", (const char*)appDataBuffer)){
        receivedLiveInput(payload);
      } else if(!strcmp("STLI", (const char *)appDataBuffer)){

        clearFrameAction();

      } else if(!strcmp("ANIM", (const char *)appDataBuffer)){

        AnimationAction((const char *)(payload+4));

      } else {
          server.sendTXT(num, payload);
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

void readFileAction( uint8_t num, uint8_t * payload  ){
  listenLiveInput = false;
  if (defaultState || AnimationRunning){
    defaultState = false;
    AnimationRunning = false;
  }
  clearBuffer(appDataBuffer,4);
  readfile(num,  (payload+4));

  strcpy((char *)filename, (const char *)(payload+4));
  strncpy((char *)stateMachine, "OPEN", 4);

  appInput = true;
}

void writeFileAction(uint8_t num, size_t length, uint8_t * payload ){
  strncpy((char *) appDataBuffer, (const char *)(payload + length - 4), 4);
  if(!strcmp("EX1T", (const char *)appDataBuffer)){
    *(payload+length-4) = '\0';
  }
  std::string suffix = (const char *)appDataBuffer;
  std::string filename = extractFilename(payload+4);
  clearBuffer(appDataBuffer,4);
  writefile(num, filename.c_str(), (payload+4+filename.length()), suffix.c_str());
}

void appendFileAction(uint8_t num, size_t length, uint8_t * payload ){
  strncpy((char *) appDataBuffer, (const char *)(payload + length - 4), 4);
  if(!strcmp("EX1T", (const char *)appDataBuffer)){
    *(payload+length-4) = '\0';
  }
  std::string suffix = (const char *)appDataBuffer;
  std::string filename = extractFilename(payload+4);
  clearBuffer(appDataBuffer,4);
  appendfile(num, filename.c_str(), (payload+4+filename.length()), suffix.c_str());
}

void liveInputState( uint8_t * payload){
  if(defaultState || AnimationRunning){
    defaultState = false;
    AnimationRunning = false;
  }
  strncpy((char *) stateMachine, "LIVE", 4);
  listenLiveInput = true;
  appInput = true;

}
void receivedLiveInput(uint8_t * payload){
  clearBuffer(appDataBuffer,strlen((const char *) payload));
  strncpy((char *)appDataBuffer, (const char*)(payload+4), strlen((const char *)payload+4));

  receivedLiveData = true;
  while(bufferLock){
    delay(5);
  }
}
void clearFrameAction(){
  // Clear the display
  appInput = true;
  if (defaultState || AnimationRunning){
    defaultState = false;
    AnimationRunning = false;
  }
  strncpy((char *) stateMachine, "CLCR", 4);

}
void exitLiveInputState() {
  if(listenLiveInput){
    listenLiveInput = false;
    receivedLiveData =false;
    defaultState = false;
    AnimationRunning = false;
  }
  strncpy((char *)stateMachine, "CLCR", 4);
  appInput = true;
}
void clearLiveInput() {
  strncpy((char *)stateMachine, "CLRI",4);
  listenLiveInput = false;
  appInput = true;
}
void AnimationAction(const char * animationLabel){
  AnimationRunning = true;
  if (defaultState || listenLiveInput){
    defaultState = false;
    listenLiveInput = false;
  }

  strncpy((char*)animation, animationLabel, 4);
  strncpy((char *)stateMachine, "ANIM", 4);

  appInput = true;
}