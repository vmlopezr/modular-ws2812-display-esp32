#include "WebServerTask.h"
#include "LedDriverTask.h"
#include "Filesystem.h"
// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(num == 0) {
    clearBuffer(appDataBuffer, MAX_BUFFER_SIZE);
    // Figure out the type of WebSocket event
    switch(type) {

      // Client has disconnected
      case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        clearBuffer(stateMachine, 4);
        strncpy((char *)stateMachine, "DEFT", 4);


        if(animationState || liveInputState){
          animationState = false;
          liveInputState = false;
        }
        matrix.resetLeds();
        defaultState = true;
        appInput = true;
        break;

      // New client has connected
      case WStype_CONNECTED:
        {
          server.sendTXT(num, "ACCEPT");
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
        } else if (!strcmp("SETT", (const char *)appDataBuffer)){
          defaultState = false;
          liveInputState = false;
          animationState = false;
          appInput = false;
          Serial.printf("Currently in settings\n");
        } else if(!strcmp("EDEF",(const char *)appDataBuffer)) {
          defaultState = false;
          liveInputState = false;
          animationState = false;
          appInput = false;
          Serial.printf("Currently in Default Input\n");
        } else if (!strcmp("read", (const char *)appDataBuffer)){

          readFileAction(num, payload);

        } else if (!strcmp("DFRD",(const char *)appDataBuffer)){
          defaultReadAction(num, payload);

        } else if (!strcmp("GTSZ",(const char *)appDataBuffer)) {
          sendSize(num);

        } else if (!strcmp("size",(const char *)appDataBuffer)){

          //TODO: IMPORTANT when size changes, move DefaultDisplay.txt, to a new file "PreviousDisplay.txt", makeDefaultDisplay.txt empty
          //File file = SD.open("/Production/DefaultDisplay.txt");
          bool sizeChanged = updateDisplaySize((char *)(payload+4), height, width);

          // Update the buffer and led arrays on size changes
          if(sizeChanged){
            matrix.updateLength(height*width);
            updateBufferLength(width, height);
            matrix.resetLeds();
            matrix.write_leds();
          }
          if(!defaultState){
            defaultState = true;
            appInput = true;
          }

        } else if(!strcmp("LIVE",(const char*)appDataBuffer)){
          // Enter liveInput State
          liveInputAction(payload);

        } else if(!strcmp("EXLI", (const char*)appDataBuffer)){
          // Exit liveInput and enter DefaultState
          exitLiveInputState();

        } else if(!strcmp("ANIM", (const char *)appDataBuffer)){
          // Enter Animation State
          Serial.printf("Animation Input: %s\n", (const char*)payload);
          AnimationAction((const char *)(payload+4));

        }else if(!strcmp("CLRI", (const char*)appDataBuffer)){

          clearLiveInput();

        } else if(!strcmp("save", (const char*)appDataBuffer)){

          writeFileAction(num, length, payload);

        } else if(!strcmp("apnd", (const char*)appDataBuffer)){

          appendFileAction(num, length, payload);

        } else if(!strcmp("SDEF", (const char*)appDataBuffer)){

          writefile("/Production/DefaultDisplay.txt", (const char *)(payload+4));
          strcpy((char *)appDataBuffer, (const char *)(payload+4));
          if(!writeDefaultFrames((const char *)appDataBuffer)){
            //Reset Default Frame Array data
            resetFrameData();
          }

        } else if (!strcmp("GDEF", (const char*)appDataBuffer)) {

          getDefaultFrames(num);

        } else if(!strcmp("EXWT",(const char*)appDataBuffer)){

          server.sendTXT(num, "SUXS");

        } else if(!strcmp("dels", (const char*)appDataBuffer)){

          deletefile((const char*)(payload+4));

        } else if(!strcmp("INPT", (const char*)appDataBuffer)){

          receivedLiveInput(payload);

        } else if(!strcmp("STLI", (const char *)appDataBuffer)){

          clearFrameAction();

        }else if(!strcmp("TYPE",(const char *)appDataBuffer)) {

          updateMatrixTypeData(payload+4);

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
  } else {
    server.sendTXT(num, "REJECT");
  }
}

bool updateDisplaySize(char * data, size_t &oldHeight, size_t &oldWidth){
  int temp;
  char *size;
  bool sizeChanged = false;

  size = strtok((char *) data, " ");
  temp = strtol((const char*)size, NULL, 10);
  size = strtok(NULL, " ");

  if(temp != oldHeight){
    oldHeight = temp;
    sizeChanged = true;
  }

  temp = strtol((const char*)size, NULL, 10);

  if( temp != oldWidth){
    oldWidth = temp;
    sizeChanged = true;
  }
  return sizeChanged;
}

void readFileAction( uint8_t num, uint8_t * payload){
  strcpy((char *)filename, (const char *)(payload+4));
  strncpy((char *)stateMachine, "OPEN", 4);
  liveInputState = false;

  while(fileLock){
    delay(1);
  }

  clearBuffer(appDataBuffer,4);
  readfile(num,  (payload+4));

  liveInputState = true;
  appInput = true;
}
void defaultReadAction(uint8_t num, uint8_t * payload){

  clearBuffer(appDataBuffer,4);
  readfile(num,  (payload+4));

  strcpy((char *)filename, (const char *)(payload+4));
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

void liveInputAction( uint8_t * payload){
  if(defaultState || animationState){
    defaultState = false;
    animationState = false;
  }
  strncpy((char *) stateMachine, "LIVE", 4);
  liveInputState = true;
  appInput = true;

}
void receivedLiveInput(uint8_t * payload){
  clearBuffer(appDataBuffer,strlen((const char *) payload));
  strncpy((char *)appDataBuffer, (const char*)(payload+4), strlen((const char *)payload+4));
  processLiveData();
  receivedLiveData = true;
}
void clearFrameAction(){
  // Clear the display

  if (liveInputState || animationState){
    liveInputState = false;
    animationState = false;
  }
  defaultState = true;
  strncpy((char *) stateMachine, "CLCR", 4);
  appInput = true;
}
void exitLiveInputState() {
  if(liveInputState){
    liveInputState = false;
    receivedLiveData = false;
    animationState = false;
  }

  strncpy((char *)stateMachine, "CLCR", 4);
  defaultState = true;
  appInput = true;
}
void clearLiveInput() {

  strncpy((char *)stateMachine, "CLRI",4);
  liveInputState = false;
  appInput = true;

}
void AnimationAction(const char * animationLabel){

  clearBuffer(animation, 4);
  strncpy((char*)animation, animationLabel, 4);
  strncpy((char *)stateMachine, "ANIM", 4);

  if (defaultState || liveInputState){
    defaultState = false;
    liveInputState = false;
  }
  animationState = true;
  appInput = true;
}
void updateMatrixTypeData( uint8_t * mType) {
  matrixType.assign((const char *)mType);
  if(!strcmp("CJMCU-64",matrixType.c_str())){
    isCJMCU = true;
  } else {
    isCJMCU = false;
  }
}


void getDefaultFrames(uint8_t client){
  clearBuffer(appDataBuffer,4);
  for(int i = 0; i < numSavedFrames; i++){
    strcat((char *)appDataBuffer, FileNames[i].c_str());
    strcat((char *)appDataBuffer, ",");
    strcat((char *)appDataBuffer, Effects[i].c_str());
    strcat((char *)appDataBuffer, ",");
    strcat((char *)appDataBuffer, DisplayTime[i].c_str());
    strcat((char *)appDataBuffer, "\n");
  }
  server.sendTXT(client,(const char *)appDataBuffer);
  server.sendTXT(client, "EX1T");
}
void sendSize(uint8_t client){
  clearBuffer(appDataBuffer, 50);
  strcat((char *)appDataBuffer, S_height.c_str());
  strcat((char *)appDataBuffer," ");
  strcat((char *)appDataBuffer, S_width.c_str());
  server.sendTXT(client, (const char *)appDataBuffer);
}
void updateBufferLength(int newWidth, int newHeight){
  delete[] LEDBuffer1;
  delete[] LEDBuffer2;
  const size_t length = newWidth*newHeight;
  try {
    LEDBuffer1 = new uint32_t[length];
    LEDBuffer2 = new uint32_t[length];
  }
    catch(std::bad_alloc){
      LEDBuffer1 = NULL;
      LEDBuffer2 = NULL;
      Serial.printf("Bad Allocation on length update for LED Buffers.\n");
    return;
  }
  memset(LEDBuffer1, 0, length);
  memset(LEDBuffer2, 0, length);
}