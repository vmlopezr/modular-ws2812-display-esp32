#include "WS2812SDCard.h"

//  /SdcardTest.txt
//  /SDcardReadTest.txt
void loadDataFromStorage(Esp32CtrlLed &Leds, const char *filename){
  File file = SD.open((const char *)filename);
  if(!file){
    Serial.printf("Failed to  open file for reading\n");
    return;
  }
    // Retrieve the size of the file
  char temp;

  // Move file pointer to the second line.
  do {
      temp = file.read();
      if( temp == '\n' || temp == '\r'){
          break;
      }
  } while (file.available());

  int char_count = 0;
  char data[6];
  size_t index = 0;
  size_t color;

  // Read the rest of the data and send it to the phone
  while(file.available()){
      if(char_count >= 6){ // Begin parsing string for color
        char_count = 0;
        color = strtol((const char *)data, NULL, 16);
        Leds.setPixelRGB(index, color);
        index++;
      }
      temp = file.read();
      if(temp != '\n' && temp != '\r' && temp != ' '){
        data[char_count] = temp;
        char_count++;
      }
  }
  file.close();
}
void loadDataToBuffer(uint32_t *buffer, const char* filename){
  File file = SD.open((const char *)filename);
  if(!file){
    Serial.printf("Failed to  open file for reading\n");
    return;
  }
    // Retrieve the size of the file
  char temp;

  // Move file pointer to the second line.
  do {
      temp = file.read();
      if( temp == '\n' || temp == '\r'){
          break;
      }
  } while (file.available());

  int char_count = 0;
  char data[6];
  size_t index = 0;

  // Read the rest of the data and send it to the phone
  while(file.available()){
      if(char_count >= 6){ // Begin parsing string for color
        char_count = 0;
        if(index <= matrix.NUM_LEDS) {
          buffer[index] = strtol((const char *)data, NULL, 16);
        }
        index++;
      }
      temp = file.read();
      if(temp != '\n' && temp != '\r' && temp != ' '){
        data[char_count] = temp;
        char_count++;
      }
  }
  file.close();
}
void writeBufferToLed(uint32_t *buffer){
  for(int i = 0; i < matrix.NUM_LEDS; i++){
    if(i >= matrix.NUM_LEDS){
      Serial.printf("equal to max size\n");
    }

    matrix.setPixelRGB(i, LEDBuffer1[i]);
  }
}