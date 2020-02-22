#include "WS2812SDCard.h"

//  /SdcardTest.txt
//  /SDcardReadTest.txt
void loadDataFromStorage(fs::FS &fs, Esp32CtrlLed &Leds, const char *filename, size_t height, size_t width){
  File file = fs.open((const char *)filename);
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
  Serial.printf("Finished color data\n");
  file.close();
}
void loadDataFromWireless(Esp32CtrlLed &Leds){}