/*    The following code has been written to control WS2812 Leds with ESP32. This library is intended for use with 8x8 matrices, but may be used
 *    with WS2812 leds in general. This was written using PlatformIO using the arduino framework for ESP32.
 *
 *    Credit to JSchaenzle: https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT - This wrapper class uses the RMT code found in the link.
 * */
#include "Esp32CtrlLed.h"

Esp32CtrlLed::Esp32CtrlLed(void) : LedData(NULL), NUM_LEDS(0), LED_CTRL_PIN(GPIO_NUM_4) {}

Esp32CtrlLed::Esp32CtrlLed(uint16_t LedNum, gpio_num_t pin) : LedData(NULL), NUM_LEDS(LedNum)
{
    updateLength(NUM_LEDS);
    setPin(pin);
}

Esp32CtrlLed::~Esp32CtrlLed()
{
    delete[] LedData;
}

/**
  @brief Set the pin number for the LED display data signal.

  @param pin Target pin for data signal.
*/
void Esp32CtrlLed::setPin(gpio_num_t pin)
{
    LED_CTRL_PIN = pin;
}

/**
  @brief Change the size of the dynamic LED array. This array holds the
  color levels to be written by the rmt peripheral.

  @param length New length of the LED array.
*/
void Esp32CtrlLed::updateLength(size_t Led_Num)
{
    delete[] LedData;
    uint32_t newSize = Led_Num * BITS_PER_LED_CMD;

    // Attempt to allocate new data array
    try
    {
        LedData = new rmt_item32_t[newSize];
    }
    catch (std::bad_alloc)
    {
        NUM_LEDS = 0;
        LED_BUFFER_SIZE = 0;
        LedData = NULL;
        Serial.printf("Bad Allocation on length update.\n");
        return;
    }
    NUM_LEDS = Led_Num;
    LED_BUFFER_SIZE = newSize;
}

/**
    @brief Initialize the RMT peripheral. This function may only be called once.
*/
void Esp32CtrlLed::ESP32_RMT_Init(void)
{
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_TX;
    config.channel = LED_RMT_TX_CHANNEL;
    config.gpio_num = LED_CTRL_PIN;
    config.mem_block_num = 3;
    config.tx_config.loop_en = false;
    config.tx_config.carrier_en = false;
    config.tx_config.idle_output_en = true;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

/**
  @brief Write the LED Data in the to the WS2812 display. The RMT peripheral
  produces the data signal.
*/
void Esp32CtrlLed::write_leds()
{
    ESP_ERROR_CHECK(rmt_write_items(LED_RMT_TX_CHANNEL, LedData, LED_BUFFER_SIZE, false));
    ESP_ERROR_CHECK(rmt_wait_tx_done(LED_RMT_TX_CHANNEL, portMAX_DELAY));
}

/**
  @brief Clear the LED display. Writes 0 for each of the RGB LED in a pixel.
*/
void Esp32CtrlLed::resetLeds()
{
    for (uint32_t led = 0; led < NUM_LEDS; led++)
    {
        setPixelRGB(led, 0);
    }
}

/**
  @brief Set the data values for the pixel at the specific index.

  @param index The index of the LED in the LED array.
  @param r The value for the red LED. ranges from 1 to 255
  @param g The value for the green LED. ranges from 1 to 255
  @param b The value for the blue LED. ranges from 1 to 255
*/
void Esp32CtrlLed::setPixelRGB(uint32_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < NUM_LEDS)
    {
        uint32_t bits_to_send = (g << 16) + (r << 8) + b;
        for (uint8_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
        {
            LedData[index * BITS_PER_LED_CMD + bit] = getNthBit(bits_to_send, 23 - bit) ? (rmt_item32_t){{{T1H, 1, T1L, 0}}} : (rmt_item32_t){{{T0H, 1, T0L, 0}}};
        }
    }
}

/**
  @brief Set the data values for the pixel at the specific index.

  @param index The index of the LED in the LED array.
  @param colorData The color for the specific pixel. Must formatted as 0x00RRGGBB
*/
void Esp32CtrlLed::setPixelRGB(uint32_t index, uint32_t colorData)
{
    if (index < NUM_LEDS)
    {
        uint32_t bits_to_send = (((colorData >> 8) & 0xFF) << 16) + (((colorData >> 16) & 0xFF) << 8) + (colorData & 0xFF);
        for (uint8_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
        {
            LedData[index * BITS_PER_LED_CMD + bit] = getNthBit(bits_to_send, 23 - bit) ? (rmt_item32_t){{{T1H, 1, T1L, 0}}} : (rmt_item32_t){{{T0H, 1, T0L, 0}}};
        }
    }
}

/**
  @brief Copy the LED pixel value of oldIndex into the new index.
  This is useful for shifting frames on the display.

  @param oldIndex Index of value to be copied.
  @param newIndex Location of the copied value.
*/
void Esp32CtrlLed::copyIndex(uint32_t oldIndex, uint32_t newIndex)
{
    for (uint8_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
    {
        LedData[newIndex * BITS_PER_LED_CMD + bit] = LedData[oldIndex * BITS_PER_LED_CMD + bit];
    }
}

/**
    @brief Gets the input bit of the number, and returns it as a base ten unsigned int.

    @param number Input value.
    @param bit Target bit to be retrieved.
    @return Bit converted to base 10 value.
*/
uint8_t Esp32CtrlLed::getNthBit(uint32_t number, uint8_t bit)
{
    return (number >> bit) & 1;
}
