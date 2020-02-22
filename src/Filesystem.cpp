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
    File file = SD.open((const char *)filename);
    char temp;

    if(!file){
        Serial.printf("Failed to open file for reading\n");
        server.sendTXT(client, "File could not be opened.");
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