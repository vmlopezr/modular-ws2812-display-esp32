#include "Filesystem.h"

void clearBuffer(uint8_t *buffer, uint16_t length){
  memset(buffer, '\0', length);
}
// Return list of all files in root directory
const char * listRootDir(fs::FS &fs, uint8_t * buffer){
    std::string DirList = "";
    uint16_t charsRead;
    File root = fs.open("/");
    if(!root){
        Serial.println("Failed to open directory");
        return "No file";
    }

    File file = root.openNextFile();
    do {
        if(!file.isDirectory()){
            DirList+=file.name();
            DirList+=",";
            charsRead = readfileSize(SD,buffer,file);
            DirList+= (const char *)buffer;
            clearBuffer(buffer, charsRead);
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
void readfile(fs::FS &fs,uint8_t *buffer, WebSocketsServer &server, uint8_t &client, uint8_t * filename){
    File file = fs.open((const char *)filename);
    char temp;
    // Serial.printf("inside read function\n");
    if(!file){
        Serial.printf("Failed to open file for reading\n");
        server.sendTXT(client, "File could not be opened.");
        return;
    }
    // Serial.printf("file opened\n");
    // Retrieve the size of the file
    unsigned int data_Available = file.available();

    // Move file point to the second line.
    do {
        temp = file.read();
        if( temp == '\n' || temp == '\r'){
            // Serial.println("read first line\n");
            break;
        }
    } while (file.available());

    data_Available = file.available();
    // Read the rest of the data and send it to the phone
    while(data_Available){
        clearBuffer(buffer, MAX_BUFFER_SIZE-1);
        if( data_Available < MAX_BUFFER_SIZE-1){
            file.read(buffer, (size_t)data_Available);
        } else {
            file.read(buffer, MAX_BUFFER_SIZE-1);
        }
        server.sendTXT(client, (const char*)buffer);
        data_Available = file.available();
    }
    /* Send "End" of data  */
    server.sendTXT(client, "EX1T");
    file.close();
}
uint16_t readfileSize(fs::FS &fs, uint8_t * buffer, File &file){
    uint16_t i =0;
    do {
        buffer[i] = file.read();
        if( buffer[i] != '\r' && buffer[i] != '\n'){
            i++;
        } else{
            buffer[i] = '\0';
            break;
        }
    } while (file.available());
    return i;
}

std::string extractFilename(uint8_t * payload){
    std::string filename;

    // Retrieve the filename
    char *occurrence = strchr((const char *)payload, '.');
    filename.assign((const char *) payload, (occurrence + 5) - (char *)(payload+1));
    return filename;
}
void writefile(fs::FS &fs, uint8_t *buffer, WebSocketsServer &server, uint8_t client, const char * filename, uint8_t * payload){

    File file = fs.open(filename, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        server.sendTXT(client, "error");
        return;
    }
    if(!file.print((const char *)payload)){
        file.close();

        Serial.printf("could not write to file\n");
        server.sendTXT(client,"writeError");
        deletefile(fs, filename);
        return;
    }
    Serial.printf("success\n");
    server.sendTXT(client, "success");

    file.close();
}
void appendfile(fs::FS &fs, uint8_t *buffer, WebSocketsServer &server, uint8_t client, const char * filename, uint8_t * payload){
    Serial.printf("Appending to file: %s\n", filename);
    File file = fs.open(filename, FILE_APPEND);

    if(!file){
        Serial.println("Failed to open file for appending");
        server.sendTXT(client, "error");
        return;
    }
    if(file.print((const char *)payload)){
        Serial.println("Message appended");
        server.sendTXT(client, "success");
    } else {
        Serial.println("Append failed");
        server.sendTXT(client,"writeError");
    }
    file.close();
}

void renamefile(fs::FS &fs, const char * path1, const char * path2){
    // Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deletefile(fs::FS &fs, const char * path){
    // Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}