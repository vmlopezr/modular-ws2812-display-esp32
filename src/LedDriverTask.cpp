#include "LedDriverTask.h"
#include "LedEffectsLib.h"
#include "WS2812SDCard.h"


/*
*  task2 to be used for the led driver code
*/
bool horizontalLine = false;
bool verticalLine = false;
bool pixel = false;

void LedDriverTask(void *parameter){

  matrix.resetLeds();
  matrix.write_leds();
  Serial.printf("Led Driver State Machine\n");

  // Set Initial state to Default Loop
  appInput = true;
  defaultState = true;
  strncpy((char *)stateMachine, "DEFT", 4);

 // Possible use of the hardware timers to keep track of the displayTime
  while(1) {
    if(appInput){
      // Default state, will be used for displaying chosen frames
      if(!strcmp("DEFT", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);
        appInput = false;

        size_t frameIndex = 0;
        size_t currentRow = 0;
        size_t currentCol = 0;
        size_t ISR_Delay = 500000; // wait one second before triggering interrupt
        // Minimum ISR Delay for sliding is 100000us ~ .1 down to 50000 ~ .05 s
        if(numSavedFrames == 1){
          loadDataFromStorage(matrix,FileNames[frameIndex].c_str());
          loadDataToBuffer(LEDBuffer1, FileNames[frameIndex].c_str());
        }

        timerAlarmWrite(timer, ISR_Delay, true);
        timerAlarmEnable(timer);
        Serial.printf("Default State\n");
        if(numSavedFrames > 0){
          while(defaultState){
            if(writeFrameISR){
              portENTER_CRITICAL(&timerMux);
              writeFrameISR = false;
              portEXIT_CRITICAL(&timerMux);

              if(frameIndex >= numSavedFrames){
                frameIndex = 0;
                // Serial.printf("Reset frame Index: %d\n", frameIndex);
              }
              if(!strcmp("None",Effects[frameIndex].c_str())){

                loadDataToBuffer(LEDBuffer1, FileNames[frameIndex].c_str());
                loadDataToBuffer(LEDBuffer2, FileNames[frameIndex].c_str());
                writeBufferToLed(LEDBuffer1);
                writeBufferToLed(LEDBuffer2);
                matrix.write_leds();
                ISR_Delay = getNumericDec(DisplayTime[frameIndex].c_str()) * 1000000;


                timerAlarmWrite(timer, ISR_Delay , true);
                frameIndex++;
              } else if (!strcmp("Horizontal Slide", Effects[frameIndex].c_str())){
                //if(!strcmp("Right", Direction[frameIndex].c_str())){
                // } else { // Going left}
                // Serial.printf("First entry in horizontal: %d\n", currentCol);

                if(currentCol == 0){
                  // if(numSavedFrames == 1){
                  //   loadDataFromStorage(matrix,FileNames[frameIndex].c_str());
                  //   loadDataToBuffer(LEDBuffer1, FileNames[frameIndex].c_str());
                  // }

                  matrix.write_leds();
                } else {
                  shiftMatrixHorizontal("Left");
                  // addColumn("Right", LEDBuffer1, width-1-currentCol);
                  addColumn("Left",LEDBuffer1, currentCol);
                  matrix.write_leds();
                }

                currentCol++;
                if(currentCol >= width ){
                  currentCol = 0;
                  frameIndex++;
                }
                // ISR_Delay = SlideSpeed[frameIndex] + factor for milli to micro seconds
                timerAlarmWrite(timer, ISR_Delay , true);
              } else if(!strcmp("Vertical Slide", Effects[frameIndex].c_str())){




                if(numSavedFrames == 1){
                  shiftMatrixVertical("Down");
                  addRow("Down", LEDBuffer1, currentRow);

                  // shiftMatrixVertical(Direction[frameIndex].c_str());
                  // addRow(Direction[frameIndex].c_str(), LEDBuffer1, currentRow);
                  // addRow("Up",LEDBuffer1, height - 1 - currentRow);
                  matrix.write_leds();
                } else {
                  // code for more frames
                  //   loadDataFromStorage(matrix,FileNames[frameIndex].c_str());
                  //   loadDataToBuffer(LEDBuffer1, FileNames[frameIndex + 1 ].c_str());
                }
                // }

                currentRow++;
                if(currentRow >= height ){
                  currentRow = 0;
                  frameIndex++;
                }
                // ISR_Delay = SlideSpeed[frameIndex] + factor for milli to micro seconds
                timerAlarmWrite(timer, ISR_Delay , true);
              } else if(!strcmp("Fade", Effects[frameIndex].c_str())){

              }


            }
            delay(1);
          }
        } else {
          while(defaultState){
            delay(1);
          }
        }
        timerAlarmDisable(timer);
      // Animation state: Display the animation chosen by the user.
      // The PIXL animation may be used for verifying LED Matrix connections.
      } else if(!strcmp("ANIM", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);
        appInput = false;
        Serial.printf("Start Animation %s\n", (const char *)animation);

        matrix.resetLeds();
        matrix.write_leds();
        // counters for the animation loop
        size_t newindex = 0;
        size_t prev = newindex;

        if(!strncmp("HLNE", (const char*) animation, 4)){
          horizontalLine = true;
          pixel = false;
          verticalLine = false;
        } else if(!strncmp("PIXL",(const char*)animation,4)){
          pixel = true;
          horizontalLine = false;
          verticalLine = false;
        } else if(!strncmp("VLNE", (const char*)animation,4)){
          pixel = false;
          horizontalLine = false;
          verticalLine = true;
        }
        while(animationState){
          // Animate a Horizontal horizontalLine moving translating from the bottom of the display to the top.
          if(horizontalLine){
            if(newindex >= height){
              newindex = 0;
              prev= height - 1;
            }
            HorizontalLine(newindex, prev, width, height, 0x000F0F00);
            matrix.write_leds();

            prev = newindex;
            newindex++;
          }
          // Animate a pixel moving on display. It follows the index count of the pcb connections.
          else if(pixel){
            if(newindex >= matrix.NUM_LEDS){
              newindex = 0;
              prev=matrix.NUM_LEDS - 1;
            }

            SinglePixel(newindex, prev, 0x000F0F00);
            matrix.write_leds();
            prev = newindex;
            newindex++;

          }
          else if(verticalLine){
            if(newindex >= width){
              newindex = 0;
              prev= width - 1;
            }
            VerticalLine(newindex, prev, width, height, 0x000F0F00);
            matrix.write_leds();

            prev = newindex;
            newindex++;
          }
          delay(50);
        }
        newindex = 0;
        prev = 0;

      // The open state will display the frame. Then change the statemachine to
      // a "listen for live input" state
      } else if(!strcmp("OPEN", (const char*)stateMachine)){
        fileLock = true;
        clearBuffer(stateMachine, 4);
        unsigned long start = micros();
        loadDataFromStorage(matrix, (const char *)filename);
        fileLock = false;

        matrix.write_leds();
        Serial.printf("read time: %lu\n", micros() - start);
        clearBuffer(filename,strlen((const char *)filename));
        if(liveInputState) {
          strncpy((char *) stateMachine, "LIVE", 4);
        }


      // Clear state: The animation and open states exit through here
      } else if(!strcmp("CLCR",(const char *) stateMachine)){
        clearBuffer(stateMachine, 4);

        matrix.resetLeds();
        matrix.write_leds();
        if(defaultState){
          strncpy((char *)stateMachine, "DEFT", 4);
        }
        appInput = true;

        // Listen for live input and then display it in the frame
      } else if(!strcmp("LIVE", (const char *)stateMachine)){
        appInput = false;
        clearBuffer(stateMachine, 4);
        Serial.printf("Live input\n");
        while(liveInputState){

          if(receivedLiveData){
            matrix.write_leds();
            receivedLiveData = false;
          }
          delay(1);
        }

      } else if (!strcmp("CLRI", (const char *)stateMachine)){
        strncpy((char *)stateMachine, "LIVE",4);
        matrix.resetLeds();
        matrix.write_leds();
        liveInputState=true;
        appInput = true;
      }
    }
    delay(1);
  }
}

// Display the pixel color received live form the application
void processLiveData() {

  char *ptr;
  size_t index;
  size_t color;
  size_t count = 0;
  ptr = strtok((char *)appDataBuffer, " ");
  index = strtol((const char *)ptr, NULL, 10);
    count++;
  while((ptr=strtok(NULL," ")) != NULL){
    ++count;

    if(count%2 == 1){ // on the od counter, ptr contains the LED index
      index = strtol((const char *)ptr, NULL, 10);
    } else { // on the even counter, ptr contains the LED color
      color = strtol((const char *) ptr, NULL, 16);
      matrix.setPixelRGB(index, color);
    }
  }
}
size_t getNumericDec(const char * number){
  return  strtol(number, NULL, 10);
}