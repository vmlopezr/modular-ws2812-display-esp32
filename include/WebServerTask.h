#ifndef WEBSERVERTASK_H
#define WEBSERVERTASK_H

#include "Globals.h"
#include "Filesystem.h"

/**
  @brief WebSocker Server event loop. Detects wireless connections from clients.
  The main events used are the following:
    - WStype_DISCONNECTED: Ran when a client disconnects.
    - WStype_CONNECTED: Ran when a new client connects to the server.
    - WStype_TEXT: Ran when the server receives data.

  Majority of the work happens on the WStype_TEXT event. The first four characters
  of the payload define the endpoint. The following are the endpoints and their
  actions are as defined:
    - "dirs": Return a list of all of the files located in the root directory of the SD card.

    - "SETT": Exit LED driver loop. The LED driver task will await for state updates.
             Used when phone application enters the settings screen.

    - "EDEF": Exit LED driver loop. The LED driver task will await for state updates.
             Used when phone application enters the "Set Default" screen.

    - "read": Reads a file specified by the client, and sends back the color data.

    - "DFRD": Reads a file specified by the clien and send back the color data.

    - "size": Receives LED display size updates, and updates global variables accordingly

    - "LIVE": Initiates a state change on the LED driver task. Triggers the Live Input state.

    - "EXLI": Exits the Live Input state and triggers the default state.

    - "ANIM": Initiates a state change on the LED driver task. Triggers the Animation state.

    - "CLRI": Clears the LED display, and keeps the state in the Live Input state.

    - "save": Saves frame data received from the phone application to the specified file.

    - "apnd": Appends data received from the phone application to the specified file.

    - "SDEF": Sets the Default Display arrays (filenames, effects, displaytime, etc)
             Used by the phone application to store the sequence of frames to be displayed
             and looped by the default state.

    - "GDEF": Retrieves the Default Display arrays (fielnames, effects, displaytime, etc)
             from the "/Production/DefaultDisplay.txt" file and sends to the client.

    - "EXWT": Used by the phone application to confirm that data write has completed.
              The application expects "SUXS" as the code reply.

    - "DELS": Deletes the specified file from the root directory in the SD card.

    - "INPT": Receives pixel data (index and color) while the LED driver is in the Live
             Input state.

    - "STLI": Clears the display, and exits forces th LED driver task to exit to the default state.

    - "TYPE": Updates the matrix type selected by the client in the phone application.

    - default: echoes back the data received.
  @param num      The client id.
  @param type:    The type of event captured. Usually is one of the three mentioned above.
  @param payload: Data received from a client
  @param length:  Length of the data received
*/
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

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
bool updateSettings(char *data, size_t &prevHeight, size_t &prevWidth, std::string &matrixType);

/**
  @brief Receives the file name to be read. Updates the state machine to read and
  display the frame, then sends the frame data to the phone application.

  @param num     The client id.
  @param payload The data received from the client.
*/
void readFileAction(uint8_t num, uint8_t *payload);

/**
  @brief Receives a filename from the client, and sends the file data back.

  @param num     The client id.
  @param payload The data received from the client.
*/
void defaultReadAction(uint8_t num, uint8_t *payload);

/**
  @brief Processes data received from the client. Write the LED data  to the
  specified filename.

  @param num      The client id.
  @param length   The length of the data received.
  @param payload  The data received from the client.
*/
void writeFileAction(uint8_t num, size_t length, uint8_t *payload);

/**
  @brief Processes data received from the client. Append the LED data  to the
  specified filename.

  @param num      The client id.
  @param length   The length of the data received.
  @param payload  The data received from the client.
*/
void appendFileAction(uint8_t num, size_t length, uint8_t *payload);

/**
  @brief Trigger a state change on the LED driver task. Updates the flags to
  enter the Live Input state.

  @param payload  The data received from the client.
*/
void liveInputAction(uint8_t *payload);

/**
  @brief Clears the LED display and returns the state to the Live Input state.
*/
void clearLiveInput();

/**
  @brief Send the LED display information, width, height and matrix type, to the
  phone application. Data is delimited with a newline character.

  @param client The client id.
*/
void sendSize(uint8_t client);

/**
  @brief Trigger a state change in the LED driver task. Exits the Live Input state,
  clears the display, and enters the default state.
*/
void exitLiveInputState();

/**
  @brief Receives data from the client during the Live Input state. Loads the
  received data into AppDataBuffer to update the display.

  @param payload  The data received from the client.
*/
void receivedLiveInput(uint8_t *payload);

/**
  @brief Triggers state change for the LED driver task. Clears the LED display
  and exits the current state to enter the "Default" state.
*/
void clearFrameAction();

/**
  @brief Receive the updated matrixType from the phone application.

  @param mType New matrixType
*/
void updateMatrixTypeData(uint8_t *mType);

/**
  @brief Triggers a state change in the LED driver task. Enters the animation state
  and assigns the animation.

  @param animationLabel The chosen animation to display.
*/
void AnimationAction(const char *animationLabel);

/**
  @brief Request to read the Default Display frame file in the "Production/" folder,
  and send the data to the phone application.

  @param client The client id.
*/
void getDefaultFrames(uint8_t client);

/**
  @brief Update the size of the LEDBuffer upon LED display size updates.

  @param newWidth The new width of the display.
  @param newHeight The new height of the display
*/
void updateBufferLength(const size_t newWidth, const size_t newHeight);
#endif