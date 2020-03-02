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

  // counters for the animation loop
  int newindex = 0;
  int prev = newindex;

  while(1) {
    if(appInput){
      // Default state, will be used for displaying chosen frames
      if(!strcmp("DEFT", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);
        Serial.printf("Default State\n");
        while(defaultState){
          delay(10);
        }

      // Animation state: Display the animation chosen by the user.
      // The PIXL animation may be used for verifying LED Matrix connections.
      } else if(!strcmp("ANIM", (const char*)stateMachine)){
        Serial.printf("Start Animation\n");
        clearBuffer(stateMachine, 4);

        if(!strcmp("HLNE", (const char*) animation)){
          horizontalLine = true;
          pixel = false;
          verticalLine = false;
        } else if(!strcmp("PIXL",(const char*)animation)){
          pixel = true;
          horizontalLine = false;
          verticalLine = false;
        } else if(!strcmp("VLNE", (const char*)animation)){
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
          if(pixel){
            if(newindex >= matrix.NUM_LEDS){
              newindex = 0;
              prev=matrix.NUM_LEDS - 1;
            }

            SinglePixel(newindex, prev, 0x000F0F00);
            matrix.write_leds();

            prev = newindex;
            newindex++;
          }
          if(verticalLine){
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

      // The open state will display the frame. Then change the statemachine to
      // a "listen for live input" state
      } else if(!strcmp("OPEN", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);

        // delay(50);
        loadDataFromStorage(SD, matrix, (const char *)filename, height, width);
        matrix.write_leds();

        clearBuffer(filename,strlen((const char *)filename));
        if(liveInputState) {
          strncpy((char *) stateMachine, "LIVE", 4);
        }
        // liveInputState = true;

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