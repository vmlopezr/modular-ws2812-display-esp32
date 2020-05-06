#ifndef LEDDRIVERTASK_H
#define LEDDRIVERTASK_H

#include "Globals.h"
#include "Filesystem.h"

/**
 * @brief Led Driver Task.
 * Runs the state machine by observing the following global flags to persist loops.
 *
 *  - appInput: boolean       -> Set when screen update is received from phone
 *                               application. Controls the outermost loop in the task.
 *  - defaultState: boolean   -> Set when on startup
 *  - animationState: boolean -> Set when user starts an animation in the animation screen.
 *  - liveInputState: boolean -> Set when the user accesses the "Draw on Billboard" screen.
 *                               The driver actively listens for updates to the display.
 *
 * The following are the states observed by the global variable "stateMachine" after
 * entering the outermost loop through the appInput flag.
 *
 *   - stateMachine:
 *     - case "DEFT": Contains a while loop controlled by "defaultState" flag. Iterates
 *                  through the default Frame list and displays them with the
 *                  corresponding effects.
 *
 *     - case "ANIM": Contains a while loop controlled by the "animationState" flag.
 *                  Displays the chosen animation on the display.
 *
 *     - case "OPEN": State used while on the Live Input screen. Reads the chosen file,
 *                  sends the frame data to the phone application, and displays the
 *                  frame on the LED display.
 *
 *     - case "CLCR": Clears the LED display. The animation and open states use this state.
 *
 *     - case "LIVE": Listens for live input from the phone application. Displays
 *                  changes in real-time as the user draws on the app grid.
 *
 *     - case "CLRI": Clears the LED display. The Live Input state uses this to clear the display
 *                  and return to the Live Input State.
 *
 * @param parameter Array of parameters entered on function call.
 */
void LedDriverTask(void *parameter);

/**
 *  @brief Receive a pixel color and position to update on the display. This is called
 *  various times in the Billboard Draw screen.
 *
 * @param receivedData Pixel data sent from the phone application.
 *  Data Format: "index color"  -> pixel index first, then 24 bit color separated by a space
 */
void processLiveData(char *receivedData);

/**
 *  @brief Runs by default on power up, as well as when the phone application
 *  is not controlling the state.
 *  Loops through the default display arrays and displays the frames
 *  with their assigned effect. Loop is interrupted when a state change is
 *  triggered by the phone application.
 *
 *  Uses a timer interrupt to trigger frame advancement.
 *
 *  Global Variables Used (all extern):
 *      - FileNames: std::string*    -> Array of filenames to display.
 *      - Effects: std::string*      -> Array of effects assigned to corresponding frame.
 *      - DisplayTime: std::string*  -> Array of display times for each frame.
 *      - Direction: std::string*    -> Array of sliding direction for the slide effect.
 *      - SlideSpeed: std::string*   -> Array of sliding speeds for each frame.
 *      - BlinkTime: std::string*    -> Array of blinking times for the blink effect.
 *      - numSavedFrames: size_t     -> Number of total frames in the display loop.
 *      - writeFrameISR: volatile bool -> Flag set by the timer ISR to advance to the next
 *                                        frame in the array and display effects.
 *      - timer: hw_timer_t *          -> Timer object which triggers ISR based on an alarm.
 *      - timerMux: portMUX_TYPE       -> Used to lock the interrupt to prevent unwanted updates.
 */
void defaultDisplayState();

/**
 * @brief Displays an animation chosen by the phone application. The following
 * are the available animations:
 *  - "HLNE": Shows a horizontal line sliding from bottom to top of the display.
 *  - "PIXL": Shows a pixel traveling on the display along its data signal path.
 *  - "VLNE": Shows a vertical line slideing from right to left on the display.
 * Contains a while loop that is only exited when a state change is triggered by
 * the phone application.
 */
void animationDisplayState();

/**
 * @brief Reads the received filename from the Websocket server
 *  and displays it to the LED display.
 * Global Variables Used:
 *   - fileLock: bool         -> Flag to prevent modifications from the Websocket Server
 *   - stateMachine: char *   -> Character array denoting the new state
 *   - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *   - filename: char *       -> Filename to read and display
 *   - liveInputState: bool   -> Flag set to enter the live input state loop
 */
void openFileState();

/**
 * @brief Used to clear the LED display. This state is entered from the from the animation
 * and open file state. It exits to the default display state.
 *
 *  - stateMachine: char *   -> Character array denoting the new state
 *  - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *  - defaultState: bool     -> Flag set to enter the default display loop
 *  - appInput: bool         -> Flag denoting there is a state change triggered by the phone
 *                          application. Allows the task to enter the statemachine loop.
 */
void clearDisplayState();

/**
 * @brief Listens for input on the flag "receivedLiveData". The Websocket server
 * listens from input from the phone application, loads data to matrix object, and
 * updates "receivedLiveData" to allow the LED driver task to display the new data.
 *
 * Global Variables Used:
 *   - stateMachine: char *   -> Character array denoting the new state
 *   - liveInputState: bool   -> Flag set to enter the LiveInput loop. Websocket task updates this.
 *   - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *   - receivedLiveData: bool => Flag set by the websocket task so signal that the
 *                               LED display must be updated.
 */
void listenLiveInputState();

/**
 * @brief State that clears the LED display and then returns back to the live input state.
 * This state is only accessible from the live input state.
 *
 * Global Variables Used:
 *   - stateMachine: char *   -> Character array denoting the new state
 *   - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *   - liveInputState: bool   -> Flag set to enter the LiveInput loop. Websocket task updates this.
 *   - appInput: bool         -> Set when screen update is received from phone
 *                               application. Controls the outermost loop in the task.
 */
void clearLiveInputDisplay();

/**
 * @brief Loads the next frame into the buffer, and the current frame into the matrix object.
 * On each completion, it shifts the displayed frame one column to left or right(depending on direction).
 * Fills the empty new column with the corresponding column of the next frame.
 *
 * Global Variables Used:
 *   - numSavedFrames: size_t -> Number of frames in the default display list.
 *   - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *   - FileNames: std::string* -> Array of the filenames to be displayed in the default display state.
 *   - LEDBuffer1: uint32_t*   -> Buffer to hold the next frame to be displayed in a
 *                                frame sequence.
 */
void shiftByColumn(const size_t currentCol, const size_t frameIndex);

/**
 * @brief Loads the next frame into the buffer, and the current frame into the matrix object.
 * On each completion, it shifts the displayed frame one column to top or bottom(depending on direction).
 * Fills the empty new row with the corresponding row of the next frame.
 *
 * Global Variables Used:
 *   - numSavedFrames: size_t -> Number of frames in the default display list.
 *   - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *   - FileNames: std::string* -> Array of the filenames to be displayed in the default display state.
 *   - LEDBuffer1: uint32_t*   -> Buffer to hold the next frame to be displayed in a
 *                                frame sequence.
 */
void shiftByRow(const size_t currentRow, const size_t frameIndex);

/**
 * @brief Toggle the display based on the "toogleBlink flag".
 * @param toggleBlink: const bool -> Flag that curresponds to current state of display.
 *                                   The flag is true when a frame is being displayed.
 *                                   The flag is false when the display is cleared.
 * @param frameIndex: const size_t -> Index of the current frame being displayed
 */
bool toggleDisplay(const bool toggleBlink, const size_t frameIndex);
/**
 * @brief Convert a string number (i.e. "12", "155", etc) into a base 10 number.
 *
 * @param number String to be converted to number.
 * @return Returns the string as a base 10 number.
 */
size_t getNumericDec(const char *number);
#endif