#include "Filesystem.h"

void clearBuffer(uint8_t *array, uint16_t length){
  memset(array, '\0', length);
}
// Return list of all files in root directory
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
std::string to_string(unsigned int number){
    std::string temp = "";
    while(number){
        temp.insert(0, 1, (char)(number%10 + 48));
        number = number / 10;
    }
    return temp;
}
void readfile(uint8_t &client, uint8_t * filename){
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

        // Move file point to the second line.
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

std::string extractFilename(uint8_t *payload){
    std::string filename;

    // Retrieve the filename
    char *occurrence = strchr((const char *)payload, '.');
    filename.assign((const char *) payload, (occurrence + 5) - (char *)(payload+1));
    return filename;
}
/* Write File*/
void writefile(uint8_t client, const char *filename, uint8_t *payload, const char *suffix){

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
    if(!strcmp("EX1T", suffix)){
        server.sendTXT(client, "SUXS");
    }
    file.close();
}
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
/* Append to file to finish writing data. */
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

void renamefile(const char *path1, const char *path2){

    if (SD.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deletefile(const char *path){
    if(SD.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}
void writeDefaultFrames(const char * defaultData){
  char *ptr;
  ptr = strtok((char *)defaultData, "\n");
  size_t arraySize = strtol((const char *)ptr,NULL, 10);
  // Need to read the size;
  ptr = strtok(NULL, "\n");
  height = strtol((const char *)ptr, NULL, 10);
  ptr = strtok(NULL, "\n");
  width = strtol((const char *)ptr, NULL, 10);

  updateFrameData(arraySize);

  for(int i = 0; i < arraySize; i++){
    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      FileNames[i].assign((const char *)ptr);
    } else return;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL) {
      Effects[i].assign((const char *)ptr);
    } else return;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL){
      DisplayTime[i].assign((const char *)ptr);
    } else return;

    ptr = strtok(NULL,"\n");
    if(ptr != NULL) {
      Delays[i].assign((const char *)ptr);
    } else return;
  }

}
void StartUpDefaultFrame() {
    File file = SD.open("/Production/DefaultDisplay.txt");
    if(!file){
        Serial.printf("Failed to open file for reading: default\n");
        return;
    }
    file.read(appDataBuffer, (size_t)file.available());
    writeDefaultFrames((const char *)appDataBuffer);
    file.close();
}
void updateFrameData(size_t arraySize){
  delete[] Effects;
  delete[] DisplayTime;
  delete[] FileNames;
  delete[] Delays;

  try {
    Effects = new std::string[arraySize];
    FileNames = new std::string[arraySize];
    DisplayTime = new std::string[arraySize];
    Delays = new std::string[arraySize];
  }
  catch(std::bad_alloc){
    Serial.printf("Bad Allocation on frame data update.\n");
    FLENGTH = 0;
    return;
  }

  FLENGTH = arraySize;
}