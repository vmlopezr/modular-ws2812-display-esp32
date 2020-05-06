#include "WebServerTask.h"
#include "LedDriverTask.h"
#include "Filesystem.h"
/**
  @brief WebSocker Server event loop. Detects wireless connections from clients.
  The main events used are the following:
    - WStype_DISCONNECTED: Ran when a client disconnects.
    - WStype_CONNECTED: Ran when a new client connects to the server.
    - WStype_TEXT: Ran when the server receives data.

  @param num      The client id.
  @param type:    The type of event captured. Usually is one of the three mentioned above.
  @param payload: Data received from a client
  @param length:  Length of the data received
*/
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    if (num == 0)
    {
        clearBuffer(appDataBuffer, MAX_BUFFER_SIZE);
        // Figure out the type of WebSocket event
        switch (type)
        {

        // Client has disconnected
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            clearBuffer(stateMachine, 4);
            strncpy((char *)stateMachine, "DEFT", 4);

            if (animationState || liveInputState)
            {
                animationState = false;
                liveInputState = false;
            }
            matrix.resetLeds();
            defaultState = true;
            appInput = true;
            break;

        // New client has connected
        case WStype_CONNECTED:
        {
            IPAddress ip = server.remoteIP(num);
            Serial.printf("[%u] Connection from ", num);
            Serial.println(ip.toString());
            sendSize(num);
        }
        break;

        // Echo text message back to client
        // Research using a dictionary like structure to minimize the else if chain.
        case WStype_TEXT:
            // Extract the first four characters of message
            strncpy((char *)appDataBuffer, (const char *)payload, 4);

            // Return a list of all of the files located in the root directory of the SD card.
            if (!strcmp("dirs", (const char *)appDataBuffer))
            {
                server.sendTXT(num, listRootDir());
            }

            // Exit LED driver loop. The LED driver task will await for state updates.
            // Used when phone application enters the settings screen.
            else if (!strcmp("SETT", (const char *)appDataBuffer))
            {
                defaultState = false;
                liveInputState = false;
                animationState = false;
                appInput = false;
            }
            // Exit LED driver loop. The LED driver task will await for state updates.
            // Used when phone application enters the "Set Default" screen.
            else if (!strcmp("EDEF", (const char *)appDataBuffer))
            {
                defaultState = false;
                liveInputState = false;
                animationState = false;
                appInput = false;
            }

            // Reads a file specified by the client, and sends back the color data.
            else if (!strcmp("read", (const char *)appDataBuffer))
            {

                readFileAction(num, payload);
            }
            // Reads a file specified by the clien and send back the color data.
            else if (!strcmp("DFRD", (const char *)appDataBuffer))
            {
                defaultReadAction(num, payload);
            }

            // Receives LED display size updates, and updates global variables accordingly
            else if (!strcmp("size", (const char *)appDataBuffer))
            {
                bool sizeChanged = updateSettings((char *)(payload + 4), height, width, matrixType);

                // Update the buffer and led arrays on size changes
                if (sizeChanged)
                {
                    matrix.updateLength(height * width);
                    updateBufferLength(width, height);
                    matrix.resetLeds();
                    matrix.write_leds();
                }
                if (!defaultState)
                {
                    defaultState = true;
                    appInput = true;
                }
            }
            // Initiates a state change on the LED driver task. Triggers the Live Input state.
            else if (!strcmp("LIVE", (const char *)appDataBuffer))
            {
                // Enter liveInput State
                liveInputAction(payload);
            }
            // Exits the Live Input state and triggers the default state.
            else if (!strcmp("EXLI", (const char *)appDataBuffer))
            {
                // Exit liveInput and enter DefaultState
                exitLiveInputState();
            }
            // Initiates a state change on the LED driver task. Triggers the Animation state.
            else if (!strcmp("ANIM", (const char *)appDataBuffer))
            {
                // Enter Animation State
                AnimationAction((const char *)(payload + 4));
            }
            // Clears the LED display, and keeps the state in the Live Input state.
            else if (!strcmp("CLRI", (const char *)appDataBuffer))
            {

                clearLiveInput();
            }
            // Saves frame data received from the phone application to the specified file.
            else if (!strcmp("save", (const char *)appDataBuffer))
            {

                writeFileAction(num, length, payload);
            }
            // Appends data received from the phone application to the specified file.
            else if (!strcmp("apnd", (const char *)appDataBuffer))
            {

                appendFileAction(num, length, payload);
            }
            //  Sets the Default Display arrays (filenames, effects, displaytime, etc)
            //  Used by the phone application to store the sequence of frames to be displayed
            //  and looped by the default state.
            else if (!strcmp("SDEF", (const char *)appDataBuffer))
            {
                if (!SD.exists("/Production"))
                {
                    SD.mkdir("/Production");
                    Serial.printf("Production Folder did not exist\n");
                }
                writefile("/Production/DefaultDisplay.txt", (const char *)(payload + 4));
                strcpy((char *)appDataBuffer, (const char *)(payload + 4));
                if (!readDefaultFrames((const char *)appDataBuffer))
                {
                    //Set Default Frame Data to default values
                    resetFrameData();
                }
            }
            // Retrieves the Default Display arrays (fielnames, effects, displaytime, etc)
            //  from the "/Production/DefaultDisplay.txt" file and sends to the client.
            else if (!strcmp("GDEF", (const char *)appDataBuffer))
            {

                getDefaultFrames(num);
            }
            // Used by the phone application to confirm that data write has completed.
            //   The application expects "SUXS" as the code reply.
            else if (!strcmp("EXWT", (const char *)appDataBuffer))
            {

                server.sendTXT(num, "SUXS");
            }
            // Deletes the specified file from the root directory in the SD card.
            else if (!strcmp("DELS", (const char *)appDataBuffer))
            {

                deletefile((const char *)(payload + 4));
            }
            // Receives pixel data (index and color) while the LED driver is in the Live
            //  Input state.
            else if (!strcmp("INPT", (const char *)appDataBuffer))
            {

                receivedLiveInput(payload);
            }
            // Clears the display, and exits forces th LED driver task to exit to the default state.
            else if (!strcmp("STLI", (const char *)appDataBuffer))
            {

                clearFrameAction();
            }
            // Updates the matrix type selected by the client in the phone application.
            else if (!strcmp("TYPE", (const char *)appDataBuffer))
            {

                updateMatrixTypeData(payload + 4);
            }
            else
            {

                server.sendTXT(num, payload);
            }

            break;

        // For everything else: do nothing
        case WStype_BIN:
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        default:
            break;
        }
    }
    else
    {
        server.sendTXT(num, "REJECT\n");
    }
}

/**
  @brief Update settings received from the phone application. These include the
  LED display height, width, and matrix type. Data received is expected in the
  following format:
    - "height width matrixType" with all values separated by spaces.

  @param data        Data received from the client.
  @param prevHeight  The current height. Passed by reference to update the value.
  @param prevWidth   The current width. Passed by reference to update the value.
  @param matrixType  The current matrix type. Passed by reference to update the value.
*/
bool updateSettings(char *data, size_t &prevHeight, size_t &prevWidth, std::string &matrixType)
{
    int temp;
    char *ptr;
    bool sizeChanged = false;

    ptr = strtok(data, " ");
    temp = strtol((const char *)ptr, NULL, 10);
    ptr = strtok(NULL, " ");

    if (temp != prevHeight)
    {
        prevHeight = temp;
        sizeChanged = true;
    }

    temp = strtol((const char *)ptr, NULL, 10);

    if (temp != prevWidth)
    {
        prevWidth = temp;
        sizeChanged = true;
    }

    ptr = strtok(NULL, " ");
    matrixType.assign((const char *)ptr);

    if (!strcmp("CJMCU-64", matrixType.c_str()))
    {
        isCJMCU = true;
    }
    else
    {
        isCJMCU = false;
    }
    return sizeChanged;
}

/**
  @brief Receives the file name to be read. Updates the state machine to read and
  display the frame, then sends the frame data to the phone application.

  @param num     The client id.
  @param payload The data received from the client.
*/
void readFileAction(uint8_t num, uint8_t *payload)
{
    strcpy((char *)filename, (const char *)(payload + 4));
    strncpy((char *)stateMachine, "OPEN", 4);
    liveInputState = false;

    // wait until lock is removed
    while (fileLock)
    {
        delay(1);
    }

    clearBuffer(appDataBuffer, 4);
    readfile(num, (payload + 4));

    liveInputState = true;
    appInput = true;
}

/**
  @brief Receives a filename from the client, and sends the file data back.

  @param num     The client id.
  @param payload The data received from the client.
*/
void defaultReadAction(uint8_t num, uint8_t *payload)
{

    clearBuffer(appDataBuffer, 4);
    readfile(num, (payload + 4));

    strcpy((char *)filename, (const char *)(payload + 4));
}

/**
  @brief Processes data received from the client. Write the LED data  to the
  specified filename.

  @param num      The client id.
  @param length   The length of the data received.
  @param payload  The data received from the client.
*/
void writeFileAction(uint8_t num, size_t length, uint8_t *payload)
{
    strncpy((char *)appDataBuffer, (const char *)(payload + length - 4), 4);
    if (!strcmp("EX1T", (const char *)appDataBuffer))
    {
        *(payload + length - 4) = '\0';
    }
    std::string msgCode = (const char *)appDataBuffer;
    std::string filename = extractFilename(payload + 4);
    clearBuffer(appDataBuffer, 4);
    writefile(num, filename.c_str(), (payload + 4 + filename.length()), msgCode.c_str());
}

/**
  @brief Processes data received from the client. Append the LED data  to the
  specified filename.

  @param num      The client id.
  @param length   The length of the data received.
  @param payload  The data received from the client.
*/
void appendFileAction(uint8_t num, size_t length, uint8_t *payload)
{
    strncpy((char *)appDataBuffer, (const char *)(payload + length - 4), 4);
    if (!strcmp("EX1T", (const char *)appDataBuffer))
    {
        *(payload + length - 4) = '\0';
    }
    std::string suffix = (const char *)appDataBuffer;
    std::string filename = extractFilename(payload + 4);
    clearBuffer(appDataBuffer, 4);
    appendfile(num, filename.c_str(), (payload + 4 + filename.length()), suffix.c_str());
}

/**
  @brief Trigger a state change on the LED driver task. Updates the flags to
  enter the Live Input state.

  @param payload  The data received from the client.
*/
void liveInputAction(uint8_t *payload)
{
    if (defaultState || animationState)
    {
        defaultState = false;
        animationState = false;
    }
    strncpy((char *)stateMachine, "LIVE", 4);
    liveInputState = true;
    appInput = true;
}

/**
  @brief Receives data from the client during the Live Input state. Loads the
  received data into AppDataBuffer to update the display.

  @param payload  The data received from the client.
*/
void receivedLiveInput(uint8_t *payload)
{
    clearBuffer(appDataBuffer, strlen((const char *)payload));
    strncpy((char *)appDataBuffer, (const char *)(payload + 4), strlen((const char *)payload + 4));
    processLiveData((char *)appDataBuffer);
    receivedLiveData = true;
}

/**
  @brief Triggers state change for the LED driver task. Clears the LED display
  and exits the current state to enter the "Default" state.
*/
void clearFrameAction()
{
    if (liveInputState || animationState)
    {
        liveInputState = false;
        animationState = false;
    }
    defaultState = true;
    strncpy((char *)stateMachine, "CLCR", 4);
    appInput = true;
}

/**
  @brief Trigger a state change in the LED driver task. Exits the Live Input state,
  clears the display, and enters the default state.
*/
void exitLiveInputState()
{
    if (liveInputState)
    {
        liveInputState = false;
        receivedLiveData = false;
        animationState = false;
    }

    strncpy((char *)stateMachine, "CLCR", 4);
    defaultState = true;
    appInput = true;
}
/**
  @brief Clears the LED display and returns the state to the Live Input state.
*/
void clearLiveInput()
{
    strncpy((char *)stateMachine, "CLRI", 4);
    liveInputState = false;
    appInput = true;
}

/**
  @brief Triggers a state change in the LED driver task. Enters the animation state
  and assigns the animation.

  @param animationLabel The chosen animation to display.
*/
void AnimationAction(const char *animationLabel)
{
    clearBuffer(animation, 4);
    strncpy((char *)animation, animationLabel, 4);
    strncpy((char *)stateMachine, "ANIM", 4);

    if (defaultState || liveInputState)
    {
        defaultState = false;
        liveInputState = false;
    }
    animationState = true;
    appInput = true;
}

/**
  @brief Receive the updated matrixType from the phone application.

  @param mType New matrixType
*/
void updateMatrixTypeData(uint8_t *mType)
{
    matrixType.assign((const char *)mType);
    if (!strcmp("CJMCU-64", matrixType.c_str()))
    {
        isCJMCU = true;
    }
    else
    {
        isCJMCU = false;
    }
}

/**
  @brief Request to read the Default Display frame file in the "Production/" folder,
  and send the data to the phone application.

  @param num The client id.
*/
void getDefaultFrames(uint8_t client)
{
    clearBuffer(appDataBuffer, 4);
    strcat((char *)appDataBuffer, S_width.c_str());
    strcat((char *)appDataBuffer, ",");
    strcat((char *)appDataBuffer, S_height.c_str());
    strcat((char *)appDataBuffer, "\n");
    for (int i = 0; i < numSavedFrames; i++)
    {
        strcat((char *)appDataBuffer, FileNames[i].c_str());
        strcat((char *)appDataBuffer, ",");
        strcat((char *)appDataBuffer, Effects[i].c_str());
        strcat((char *)appDataBuffer, ",");
        strcat((char *)appDataBuffer, DisplayTime[i].c_str());
        strcat((char *)appDataBuffer, ",");
        strcat((char *)appDataBuffer, Direction[i].c_str());
        strcat((char *)appDataBuffer, ",");
        strcat((char *)appDataBuffer, SlideSpeed[i].c_str());
        strcat((char *)appDataBuffer, ",");
        strcat((char *)appDataBuffer, BlinkTime[i].c_str());
        strcat((char *)appDataBuffer, "\n");
    }
    server.sendTXT(client, (const char *)appDataBuffer);
    server.sendTXT(client, "EX1T");
}
/**
  @brief Send the LED display information, width, height and matrix type, to the
  phone application. Data is delimited with a newline character.

  @param client The client id.
*/
void sendSize(uint8_t client)
{
    clearBuffer(appDataBuffer, 100);
    strcat((char *)appDataBuffer, "ACCEPT\n");
    strcat((char *)appDataBuffer, S_height.c_str());
    strcat((char *)appDataBuffer, "\n");
    strcat((char *)appDataBuffer, S_width.c_str());
    strcat((char *)appDataBuffer, "\n");
    strcat((char *)appDataBuffer, matrixType.c_str());
    strcat((char *)appDataBuffer, "\n");

    for (int i = 0; i < numSavedFrames; i++)
    {
        strcat((char *)appDataBuffer, FileNames[i].c_str());
        if (i != numSavedFrames - 1)
            strcat((char *)appDataBuffer, ",");
    }
    server.sendTXT(client, (const char *)appDataBuffer);
}
/**
  @brief Update the size of the LEDBuffer upon LED display size updates.

  @param newWidth The new width of the display.
  @param newHeight The new height of the display
*/
void updateBufferLength(const size_t newWidth, const size_t newHeight)
{
    delete[] LEDBuffer1;
    LEDBuffer1 = NULL;

    const size_t length = newWidth * newHeight;
    try
    {
        LEDBuffer1 = new uint32_t[length];
    }
    catch (std::bad_alloc)
    {
        LEDBuffer1 = NULL;
        Serial.printf("Bad Allocation on length update for LED Buffers.\n");
        return;
    }
    memset(LEDBuffer1, 0, length);
}