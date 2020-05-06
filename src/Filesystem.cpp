#include "Filesystem.h"

/**
  @brief Clear a contiguous buffer by refilling with the null character.

  @param buffer Array to be cleared
  @param length Number of entries  to clear
*/
void clearBuffer(uint8_t *buffer, uint16_t length){
  memset(buffer, '\0', length);
}

/**
  @brief  Retrieve a file list of root directory.
  @return An array of all files in the root directory of the SD card
*/
const char * listRootDir(){
    std::string DirList = "";
    uint16_t charsRead;

    File root = SD.open("/");
    if(!root){
        Serial.println("Failed to open directory");
        return "No file";
    }

    File file = root.openNextFile();
    do {
        if(!file.isDirectory()){
            DirList+=file.name();
            DirList+=",";
            charsRead = readfileSize(file);
            DirList+= (const char *)appDataBuffer;
            clearBuffer(appDataBuffer, charsRead);
            DirList+=",";
        }
        file = root.openNextFile();
    } while(file);
    file.close();
    root.close();

    // remove last comma
    DirList.pop_back();

    return DirList.c_str();
}

/**
  @brief Convert a unsigned integer to a string.
  @param number Input number
  @return String version of the input number. 34 -> "34"
*/
std::string to_string(unsigned int number){
    std::string temp = "";
    while(number){
        temp.insert(0, 1, (char)(number%10 + 48));
        number = number / 10;
    }
    return temp;
}

/**
  @brief Read file and and send data to client
  global vars used: WebSocketsServer server

  @param client code corresponding to client
  @param filename file to read
*/
void readfile(const uint8_t client, uint8_t * filename){
    if(strlen((const char *)filename) > 3) {
        File file = SD.open((const char *)filename);
        char temp;

        if(!file){
            Serial.printf("Failed to open file for reading: %s\n",(const char *)filename);
            server.sendTXT(client, "File could not be opened.");
            server.sendTXT(client, "EX1T");
            return;
        }
        // Retrieve the size of the file
        unsigned int data_Available = file.available();

        // Move file pointer to the second line.
        do {
            temp = file.read();
            if( temp == '\n' || temp == '\r'){
                break;
            }
        } while (file.available());

        data_Available = file.available();

        // Read the rest of the data and send it to the phone
        while(data_Available){
            clearBuffer(appDataBuffer, MAX_BUFFER_SIZE-1);
            if( data_Available < MAX_BUFFER_SIZE-1){
                file.read(appDataBuffer, (size_t)data_Available);
            } else {
                file.read(appDataBuffer, MAX_BUFFER_SIZE-1);
            }
            server.sendTXT(client, (const char*)appDataBuffer);
            data_Available = file.available();
        }
        /* Send "End" of data  */
        server.sendTXT(client, "EX1T");
        file.close();
    }
}

/**
  @brief The first line of each file contains the amount of characters in the file.
         Read the first line and return it.

  @param fileptr Pointer to the given file.
  @return File size, in characters, for the given file pointer.
*/
uint16_t readfileSize(File &file){
    uint16_t i =0;
    do {
        appDataBuffer[i] = file.read();
        if( appDataBuffer[i] != '\r' && appDataBuffer[i] != '\n'){
            i++;
        } else{
            appDataBuffer[i] = '\0';
            break;
        }
    } while (file.available());
    return i;
}

/**
  @brief Extract a filename from a data buffer. Looks for the first '.' in the
         buffer.
  @param payload Data buffer received from Websocket Server
  @return Extracted filename, contains up to three characters after the '.' found.
          Default return: "default.txt"  if no '.' found in buffer.
*/
std::string extractFilename(uint8_t *payload){
    std::string filename;

    // acquire the possition of the period that denotes the file extension
    char *occurrence = strchr((const char *)payload, '.');
    if (occurrence == NULL) return "default.txt";

    // save the filename plus the 3 characters (file extension) after the period.
    filename.assign((const char *) payload, (occurrence + 5) - (char *)(payload+1));
    return filename;
}

/**
  @brief Write payload data to a file in the SD card root directory

  @param client Code number for the client.
  @param filename Name of the file with no file extension
  @param payload Data received from client
  @param eofCode File extension
*/
void writefile(uint8_t client, const char *filename, uint8_t *payload, const char *eofCode){

    File file = SD.open(filename, FILE_WRITE);

    if(!file){
        Serial.println("Failed to open file for writing");
        server.sendTXT(client, "FERR");
        return;
    }
    if(!file.print((const char *)payload)){
        file.close();
        Serial.printf("could not write to file\n");
        server.sendTXT(client,"WERR");
        deletefile(filename);
        return;
    }
    if(!strcmp("EX1T", eofCode)){
        server.sendTXT(client, "SUXS");
    }
    file.close();
}

/**
  @brief Write payload data to a file in the SD card root directory

  @param filename Name of the file with no file extension
  @param payload Data received from client
*/
void writefile(const char *filename, const char *payload){

    File file = SD.open(filename, FILE_WRITE);

    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(!file.print((const char *)payload)){
        file.close();
        Serial.printf("could not write to file\n");
        deletefile(filename);
        return;
    }
    file.close();
}

/**
  @brief Append data to existing file.

  @param client Code number for the client.
  @param filename Name of the file to be written.
  @param payload Data received from the client.
  @param eofCode End of file code
*/
void appendfile(uint8_t client, const char *filename, uint8_t *payload, const char *suffix){
    File file = SD.open(filename, FILE_APPEND);

    if(!file){
        Serial.println("Failed to open file for appending");
        server.sendTXT(client, "FERR");
        return;
    }
    if(!file.print((const char *)payload)){
        Serial.println("Append failed");
        file.close();
        server.sendTXT(client,"WERR");
        return;
    }
    if(!strcmp("EX1T", suffix)){
        server.sendTXT(client, "SUXS");
    }
    file.close();
}

/**
  @brief Rename a file.

  @param path1 File to be renamed
  @param path2 New name for the file.
*/
void renamefile(const char *path1, const char *path2){
    if (SD.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

/**
  @brief Delete the file in the SD card.

  @param path File to be deleted
*/
void deletefile(const char *path){
    if(SD.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

/**
  @brief Loads global variables with the frame sequence to be displayed by default.
         Each line in the file corresponds to a global variable.
  @param defaultData The content of the "DefaultDisplay.txt" file.
  @return Boolean denoting the success of the read. "false" if errors occur, "true" otherwise.
*/
bool readDefaultFrames(const char * defaultData){
  char *ptr;
  ptr = strtok((char *)defaultData, "\n");

  numSavedFrames = strtol((const char *)ptr,NULL, 10);

  ptr = strtok(NULL, "\n");
  if(ptr != NULL){
    S_height.assign((const char *)ptr);
    height = strtol((const char *)ptr, NULL, 10);
  } else return false;

  ptr = strtok(NULL, "\n");
  if(ptr != NULL) {
    S_width.assign((const char *)ptr);
    width = strtol((const char *)ptr, NULL, 10);
  } else return false;

  ptr = strtok(NULL, "\n");
  if(ptr != NULL) {
    matrixType.assign((const char *)ptr);
  } else return false;

  if(numSavedFrames == 0){
      // return when no frames to display by default
      return false;
  }

  // Resize frame arrays
  updateFrameData(numSavedFrames);

  for(int i = 0; i < numSavedFrames; i++){
    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      FileNames[i].assign((const char *)ptr);
    } else return false;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL) {
      Effects[i].assign((const char *)ptr);
    } else return false;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      DisplayTime[i].assign((const char *)ptr);
    } else return false;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      Direction[i].assign((const char *)ptr);
    } else return false;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      SlideSpeed[i].assign((const char *)ptr);
    } else return false;
    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      BlinkTime[i].assign((const char *)ptr);
    } else return false;
  }
  return true;
}

/**
    @brief Encodes a single digit of a POSTNET "A" bar code.

    @param digit the single digit to encode.
    @return a bar code of the digit using "|" as the long
    bar and "," as the half bar.
*/
void defaultInitialization() {
    height = 8;
    S_height = "8";
    width = 8;
    S_width = "8";
    numSavedFrames = 0;
    Effects= NULL;
    DisplayTime= NULL;
    FileNames= NULL;
    Direction = NULL;
    SlideSpeed = NULL;
    BlinkTime = NULL;
}

/**
  @brief Read "DefaultDisplay.txt" in the Production folder. On successful read
         it sends the data to readDefaultFrames to load data to global variables.

         Loads global variables with default values in the event of incorrect "DefaultDisplay.txt"
         data format.
*/
void StartUpDefaultFrame() {
    File file = SD.open("/Production/DefaultDisplay.txt");
    if(!file){
        Serial.printf("Failed to open file for reading: default\n");
        defaultInitialization();
        return;
    }
    if(file.available() < 5){
        Serial.printf("Necessary test not found in the file. The file is probably corrupted.\n");
        defaultInitialization();
        return;
    }
    file.read(appDataBuffer, (size_t)file.available());
    readDefaultFrames((const char *)appDataBuffer);
    file.close();
}

/**
  @brief Clear the global std::string* arrays for default display loop.
*/
void resetFrameData(){
  delete[] Effects;
  delete[] DisplayTime;
  delete[] FileNames;
  delete[] Direction;
  delete[] SlideSpeed;
  delete[] BlinkTime;
  numSavedFrames = 0;
  Effects =  NULL;
  DisplayTime =  NULL;
  FileNames =  NULL;
  Direction = NULL;
  SlideSpeed = NULL;
  BlinkTime = NULL;
}

/**
  @brief Resize the dynamic global arrays. Function is used when user changes display
  size on the phone application.

  @param arraySize The new size of the default display std::string* arrays.
*/
void updateFrameData(size_t arraySize){
  delete[] Effects;
  delete[] DisplayTime;
  delete[] FileNames;
  delete[] Direction;
  delete[] SlideSpeed;
  delete[] BlinkTime;
  try {
    Effects = new std::string[arraySize];
    FileNames = new std::string[arraySize];
    DisplayTime = new std::string[arraySize];
    Direction = new std::string[arraySize];
    SlideSpeed = new std::string[arraySize];
    BlinkTime = new std::string[arraySize];
  }
  catch(std::bad_alloc){
    Serial.printf("Bad Allocation on frame data update.\n");
    numSavedFrames = 0;
    Effects =  NULL;
    DisplayTime =  NULL;
    FileNames =  NULL;
    Direction = NULL;
    SlideSpeed = NULL;
    BlinkTime = NULL;
    return;
  }

  numSavedFrames = arraySize;
}

/**
  @brief Clear the LED display buffer.

  @param buffer The array to clear.

*/
void resetBuffer(uint32_t * buffer){
    memset(buffer, 0, matrix.NUM_LEDS);
}