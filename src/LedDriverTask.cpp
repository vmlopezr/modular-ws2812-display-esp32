#include "LedDriverTask.h"
#include "LedEffectsLib.h"
#include "WS2812SDCard.h"
/*
*  task2 to be used for the led driver code
*/
void LedDriverTask(void *parameter){

  matrix.resetLeds();
  matrix.write_leds();


  Serial.printf("Led Driver State Machine\n");


  int newindex = 0;
  int prev = newindex;

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

        while(AnimationRunning){
          if(newindex >= 8){
            newindex = 0;
            prev=7;
          }
          HorizontalLine(matrix, newindex, prev, 24, 8, 0x00010100);
          // SinglePixel(newindex, prev, 0x000F0F00);
          matrix.write_leds();

          prev = newindex;
          newindex++;
          delay(100);


        }


        // The open state will display the frame. Then change the statemachine to a "listen for live input" state
      } else if(!strcmp("OPEN", (const char*)stateMachine)){
        clearBuffer(stateMachine, 4);

        delay(50);
        loadDataFromStorage(SD, matrix, (const char *)filename, height, width);
        matrix.write_leds();

        // Serial.printf("filename: %s, height: %d, width: %d\n",(const char *)filename, height, width);

        clearBuffer(filename,strlen((const char *)filename));
        strncpy((char *) stateMachine, "LIVE", 4);
        listenLiveInput = true;

      } else if(!strcmp("CLCR",(const char *) stateMachine)){
        clearBuffer(stateMachine, 4);

        matrix.resetLeds();
        matrix.write_leds();
        Serial.printf("Clearing display\n");
        appInput = false;


        // Listen for live input and then display it in the frame
      } else if(!strcmp("LIVE", (const char *)stateMachine)){
        clearBuffer(stateMachine, 4);
        Serial.printf("listening for live input\n");
        while(listenLiveInput){
          delay(100);

        }
      }

    }
    delay(100);

  }
}