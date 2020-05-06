#include "LedDriverTask.h"
#include "LedEffectsLib.h"
#include "WS2812SDCard.h"

/*
*  task2 to be used for the led driver code
*/
bool horizontalLine = false;
bool verticalLine = false;
bool pixel = false;

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
void LedDriverTask(void *parameter)
{

    matrix.resetLeds();
    matrix.write_leds();

    // Set Initial state to Default Loop
    appInput = true;
    defaultState = true;
    strncpy((char *)stateMachine, "DEFT", 4);

    // Possible use of the hardware timers to keep track of the displayTime
    while (1)
    {
        if (appInput)
        {
            // Default state, will be used for displaying chosen frames
            if (!strcmp("DEFT", (const char *)stateMachine))
            {
                defaultDisplayState();
            }
            // Animation state: Display the animation chosen by the user.
            // The PIXL animation may be used for verifying LED Matrix connections.
            else if (!strcmp("ANIM", (const char *)stateMachine))
            {

                animationDisplayState();
            }
            // The open state will display the frame. Then change the statemachine to
            // a "listen for live input" state
            else if (!strcmp("OPEN", (const char *)stateMachine))
            {
                openFileState();
            }
            // Clear state: The animation and open states exit through here
            else if (!strcmp("CLCR", (const char *)stateMachine))
            {
                clearDisplayState();
                // Listen for live input and then display it in the frame
            }
            else if (!strcmp("LIVE", (const char *)stateMachine))
            {
                void listenLiveInputState();
            }
            else if (!strcmp("CLRI", (const char *)stateMachine))
            {
                void clearLiveInputDisplay();
            }
        }
        delay(1);
    }
}

/**
 *  @brief Receive a pixel color and position to update on the display. This is called
 *  various times in the Billboard Draw screen.
 *
 * @param receivedData Pixel data sent from the phone application.
 *  Data Format: "index color"  -> pixel index first, then 24 bit color separated by a space
 */
void processLiveData(char *receivedData)
{
    char *ptr;
    size_t index;
    size_t color;
    size_t count = 1;
    ptr = strtok(receivedData, " ");
    index = strtol((const char *)ptr, NULL, 10);
    count++;
    while ((ptr = strtok(NULL, " ")) != NULL)
    {
        // Receive pixel index on odd count, convert to decimal value
        if (count % 2 == 1)
        {
            index = strtol((const char *)ptr, NULL, 10);
        }
        // Receive pixel color on even count, convert to hex value
        else
        {
            color = strtol((const char *)ptr, NULL, 16);
            matrix.setPixelRGB(index, color);
        }
        count++;
    }
}
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
void defaultDisplayState()
{
    clearBuffer(stateMachine, 4);
    appInput = false;
    bool startBlink = false;
    bool toggleBlink = false;
    size_t blinkTotalTime = 0;
    size_t frameIndex = 0;
    size_t currentRow = 0;
    size_t currentCol = 0;
    size_t ISR_Delay = 500000; // initial timer ISR delay, in microseconds

    // Wait a half second before starting the timer
    timerAlarmWrite(timer, ISR_Delay, true);
    timerAlarmEnable(timer);
    if (numSavedFrames > 0)
    {
        // Load the first frame to the buffer and to the display array
        if (numSavedFrames == 1)
        {
            loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
            loadDataToBuffer(LEDBuffer1, FileNames[frameIndex].c_str());
        }
        while (defaultState)
        {
            // Start display update after timer ISR completes.
            if (writeFrameISR)
            {
                portENTER_CRITICAL(&timerMux);
                writeFrameISR = false;
                portEXIT_CRITICAL(&timerMux);

                // Reset the frameIndex to loop back to the beginning
                if (frameIndex >= numSavedFrames)
                {
                    frameIndex = 0;
                }

                // Detect the effect, and display
                if (!strcmp("None", Effects[frameIndex].c_str()))
                {
                    ISR_Delay = getNumericDec(DisplayTime[frameIndex].c_str()) * 1000;
                    timerAlarmWrite(timer, ISR_Delay, true);
                    loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
                    matrix.write_leds();

                    frameIndex++;
                }
                else if (!strcmp("Horizontal Slide", Effects[frameIndex].c_str()))
                {
                    shiftByColumn(currentCol, frameIndex);
                    currentCol++;
                    if (currentCol >= width)
                    {
                        currentCol = 0;
                        frameIndex++;
                    }
                }
                else if (!strcmp("Vertical Slide", Effects[frameIndex].c_str()))
                {
                    shiftByRow(currentRow, frameIndex);
                    currentRow++;
                    if (currentRow >= height)
                    {
                        currentRow = 0;
                        frameIndex++;
                    }
                }
                else if (!strcmp("Blink", Effects[frameIndex].c_str()))
                {
                    // BlinkTime[] values are in seconds. Convert to microseconds.
                    size_t displayTime = getNumericDec(DisplayTime[frameIndex].c_str()) * 1000000;
                    size_t blinkTime = getNumericDec(BlinkTime[frameIndex].c_str()) * 1000000;
                    if (!startBlink)
                    {
                        blinkTotalTime = 0;
                        startBlink = true;
                    }
                    // Set the timer delay
                    ISR_Delay = (blinkTime >= displayTime) ? displayTime : blinkTime;
                    blinkTotalTime += ISR_Delay;

                    // Set the timer alarm
                    timerAlarmWrite(timer, ISR_Delay, true);
                    toggleBlink = toggleDisplay(toggleBlink, frameIndex);

                    // Check timing condition for advancing to next frame
                    if (blinkTotalTime >= displayTime)
                    {
                        frameIndex++;
                        startBlink = false;
                        toggleBlink = false;
                    }
                }
            }
            delay(1);
        }
    }
    // When there are no default frames, do nothing. Wait for state change.
    else
    {
        while (defaultState)
        {
            delay(1);
        }
    }
    timerAlarmDisable(timer);
}

/**
 * @brief Displays an animation chosen by the phone application. The following
 * are the available animations:
 *  - "HLNE": Shows a horizontal line sliding from bottom to top of the display.
 *  - "PIXL": Shows a pixel traveling on the display along its data signal path.
 *  - "VLNE": Shows a vertical line slideing from right to left on the display.
 * Contains a while loop that is only exited when a state change is triggered by
 * the phone application.
 */
void animationDisplayState()
{
    clearBuffer(stateMachine, 4);
    appInput = false;

    matrix.resetLeds();
    matrix.write_leds();
    // counters for the animation loop
    size_t newindex = 0;
    size_t prev = newindex;

    // Set flags based on the animation chosen by phone application.
    if (!strncmp("HLNE", (const char *)animation, 4))
    {
        horizontalLine = true;
        pixel = false;
        verticalLine = false;
    }
    else if (!strncmp("PIXL", (const char *)animation, 4))
    {
        pixel = true;
        horizontalLine = false;
        verticalLine = false;
    }
    else if (!strncmp("VLNE", (const char *)animation, 4))
    {
        pixel = false;
        horizontalLine = false;
        verticalLine = true;
    }

    while (animationState)
    {
        // Animate a Horizontal horizontalLine moving translating from the bottom of the display to the top.
        if (horizontalLine)
        {
            if (newindex >= height)
            {
                newindex = 0;
                prev = height - 1;
            }
            HorizontalLine(newindex, prev, width, height, 0x000F0F00);
            matrix.write_leds();

            prev = newindex;
            newindex++;
        }
        // Animate a pixel moving on display. It follows the index count of the pcb connections.
        else if (pixel)
        {
            if (newindex >= matrix.NUM_LEDS)
            {
                newindex = 0;
                prev = matrix.NUM_LEDS - 1;
            }

            SinglePixel(newindex, prev, 0x000F0F00);
            matrix.write_leds();
            prev = newindex;
            newindex++;
        }
        else if (verticalLine)
        {
            if (newindex >= width)
            {
                newindex = 0;
                prev = width - 1;
            }
            VerticalLine(newindex, prev, width, height, 0x000F0F00);
            matrix.write_leds();

            prev = newindex;
            newindex++;
        }
        delay(10);
    }
    newindex = 0;
    prev = 0;
}

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
void openFileState()
{
    fileLock = true;
    clearBuffer(stateMachine, 4);
    loadDataToMatrixDisplay(matrix, (const char *)filename);
    fileLock = false;
    matrix.write_leds();

    clearBuffer(filename, strlen((const char *)filename));
    if (liveInputState)
    {
        strncpy((char *)stateMachine, "LIVE", 4);
    }
}

/**
 * @brief Used to clear the LED display. This state is entered from the from the animation
 * and open file state. It exits to the default display state.
 *
 *  - stateMachine: char *   -> Character array denoting the new state
 *  - matrix: Esp32CtrlLed   -> Object used to output data signal for the LED display
 *                               Contains an array that stores the pixel data
 *  - defaultState: bool     -> Flag set to enter the defaul display loop
 *  - appInput: bool         -> Flag denoting there is a state change triggered by the phone
 *                          application. Allows the task to enter the statemachine loop.
 */
void clearDisplayState()
{
    clearBuffer(stateMachine, 4);

    matrix.resetLeds();
    matrix.write_leds();
    if (defaultState)
    {
        strncpy((char *)stateMachine, "DEFT", 4);
    }
    appInput = true;
}

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
void listenLiveInputState()
{
    appInput = false;
    clearBuffer(stateMachine, 4);
    while (liveInputState)
    {
        // Websocket task receives data, loads it into matrix
        // and then sets "receivedLivedata" to update display
        if (receivedLiveData)
        {
            matrix.write_leds();
            receivedLiveData = false;
        }
        delay(1);
    }
}

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
void clearLiveInputDisplay()
{
    // Set new state for live input
    strncpy((char *)stateMachine, "LIVE", 4);
    // Clear the display
    matrix.resetLeds();
    matrix.write_leds();
    liveInputState = true;
    appInput = true;
}

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
void shiftByColumn(const size_t currentCol, const size_t frameIndex)
{
    size_t ISR_Delay = getNumericDec(SlideSpeed[frameIndex].c_str()) * 1000;
    if (numSavedFrames > 1 && currentCol == 0)
    {
        // Load the next frame into the buffer
        if (frameIndex == 0 || frameIndex < numSavedFrames - 1)
        {
            loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
            loadDataToBuffer(LEDBuffer1, FileNames[frameIndex + 1].c_str());
        }
        // Load the first frame into the buffer
        else if (frameIndex == numSavedFrames - 1)
        {
            loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
            loadDataToBuffer(LEDBuffer1, FileNames[0].c_str());
        }
    }
    shiftMatrixHorizontal(Direction[frameIndex].c_str());
    addColumn(Direction[frameIndex].c_str(), LEDBuffer1, currentCol);
    matrix.write_leds();

    timerAlarmWrite(timer, ISR_Delay, true);
}
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
void shiftByRow(const size_t currentRow, const size_t frameIndex)
{
    if (numSavedFrames > 1 && currentRow == 0)
    {
        if (frameIndex == 0 || frameIndex < numSavedFrames - 1)
        {
            loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
            loadDataToBuffer(LEDBuffer1, FileNames[frameIndex + 1].c_str());
        }
        else if (frameIndex == numSavedFrames - 1)
        {
            loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
            loadDataToBuffer(LEDBuffer1, FileNames[0].c_str());
        }
    }

    shiftMatrixVertical(Direction[frameIndex].c_str());
    addRow(Direction[frameIndex].c_str(), LEDBuffer1, currentRow);
    matrix.write_leds();

    size_t ISR_Delay = getNumericDec(SlideSpeed[frameIndex].c_str()) * 1000;
    timerAlarmWrite(timer, ISR_Delay, true);
}

/**
 * @brief Toggle the display based on the "toogleBlink flag".
 * @param toggleBlink: const bool -> Flag that curresponds to current state of display.
 *                                   The flag is true when a frame is being displayed.
 *                                   The flag is false when the display is cleared.
 * @param frameIndex: const size_t -> Index of the current frame being displayed
 */
bool toggleDisplay(const bool toggleBlink, const size_t frameIndex)
{
    // Load frame to the display
    if (!toggleBlink)
    {
        loadDataToMatrixDisplay(matrix, FileNames[frameIndex].c_str());
        matrix.write_leds();
    }
    // Clear the display
    else
    {
        matrix.resetLeds();
        matrix.write_leds();
    }
    return !toggleBlink;
}
/**
 * @brief Convert a string number (i.e. "12", "155", etc) into a base 10 number.
 *
 * @param number String to be converted to number.
 * @return Returns the string as a base 10 number.
 */
size_t getNumericDec(const char *number)
{
    return strtol(number, NULL, 10);
}