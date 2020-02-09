#include "Filesystem.h"

void clearBuffer(uint8_t *buffer, uint8_t length){
  memset(buffer, '\0', length);
}
// Return list of all files in root directory
const char * listRootDir(fs::FS &fs){
    std::string DirList = "";
    File root = fs.open("/");
    if(!root){
        Serial.println("Failed to open directory");
        return "\0";
    }
    File file = root.openNextFile();
    do {
        if(!file.isDirectory()){
            Serial.printf("%s ",file.name());
            DirList.append(file.name());
            DirList.append(",");
        }
        file = root.openNextFile();
    } while(file);
    Serial.printf("\n");
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

    // Retrieve the size of the file
    unsigned int data_Available = file.available();

    if(!file){
        Serial.println("Failed to open file for reading");
        server.sendTXT(client, "File could not be opened.");
        return;
    }
    server.sendTXT(client, to_string(data_Available).c_str());

    while(data_Available){
        clearBuffer(buffer, MAX_BUFFER_SIZE-1);
        if( data_Available < MAX_BUFFER_SIZE-1){
            file.read(buffer, (size_t)data_Available);
        } else {
            file.read(buffer, MAX_BUFFER_SIZE-1);
        }
        server.sendTXT(client, (const char*)buffer);
        data_Available = file.available();
        Serial.printf("sending data\n");
    }
    /* Send "End" of data  */
    server.sendTXT(client, "EX1T");
    file.close();
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
        server.sendTXT(client, "Failed to open file for writing");
        return;
    }
    if(!file.print((const char *)payload)){
        Serial.printf("could not write to file\n");
        server.sendTXT(client,"Could not write the file");
    }

    server.sendTXT(client, "Finished writing file");

    file.close();
}
// void appendfile(fs::FS &fs, const char * path, const char * message){
//     Serial.printf("Appending to file: %s\n", path);

//     File file = fs.open(path, FILE_APPEND);
//     if(!file){
//         Serial.println("Failed to open file for appending");
//         return;
//     }
//     if(file.print(message)){
//         Serial.println("Message appended");
//     } else {
//         Serial.println("Append failed");
//     }
//     file.close();
// }

void renamefile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deletefile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}