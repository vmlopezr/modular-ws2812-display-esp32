/*    The following code has been written to control WS2812 Leds. This library is
 *    intended for use with 8x8 matrices, but may be used with the leds in general.
 *    This was written using PlatformIO using the arduino framework for ESP32.
 *
 *    Credit to JSchaenzle: https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT -
 *    This wrapper class uses the RMT code found in the link.
 * */

#ifndef EESP32CTRLLED_H
#define EESP32CTRLLED_H

#include "driver/rmt.h"
#include "Arduino.h"

#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED_RMT_TX_GPIO GPIO_NUM_26
#define BITS_PER_LED_CMD 24

#define T0H 16 // 0 bit high ticks at  40MHZ  ~ about 400 ns
#define T0L 36 // 0 bit low  ticks at  40MHZ  ~ about 900 ns
#define T1H 36 // 1 bit high ticks at  40MHZ  ~ about 900 ns
#define T1L 24 // 1 bit low  ticks at  40MHZ  ~ about 600 ns

class Esp32CtrlLed
{
public:
    rmt_item32_t *LedData;
    uint32_t NUM_LEDS;
    size_t LED_BUFFER_SIZE;
    gpio_num_t LED_CTRL_PIN;

    Esp32CtrlLed(void);
    Esp32CtrlLed(uint16_t NUM_LEDS, gpio_num_t pin);
    ~Esp32CtrlLed();

    /**
    @brief Set the pin number for the LED display data signal.

    @param pin Target pin for data signal.
  */
    void setPin(gpio_num_t pin);

    /**
    @brief Change the size of the dynamic LED array. This array holds the
    color levels to be written by the rmt peripheral.

    @param length New length of the LED array.
  */
    void updateLength(uint32_t length);

    /**
      @brief Gets the input bit of the number, and returns it as a base ten unsigned int.

      @param number Input value.
      @param bit Target bit to be retrieved.
      @return Bit converted to base 10 value.
  */
    uint8_t getNthBit(uint32_t number, uint8_t bit);

    /**
      @brief Initialize the RMT peripheral. This function may only be called once.
  */
    void ESP32_RMT_Init(void);

    /**
    @brief Write the LED Data in the to the WS2812 display. The RMT peripheral
    produces the data signal.
  */
    void write_leds();

    /**
    @brief Clear the LED display. Writes 0 for each of the RGB LED in a pixel.
  */
    void resetLeds();

    /**
    @brief Set the data values for the pixel at the specific index.

    @param index The index of the LED in the LED array.
    @param r The value for the red LED. ranges from 1 to 255
    @param g The value for the green LED. ranges from 1 to 255
    @param b The value for the blue LED. ranges from 1 to 255
  */
    void setPixelRGB(uint32_t index, uint8_t r, uint8_t g, uint8_t b);

    /**
    @brief Set the data values for the pixel at the specific index.

    @param index The index of the LED in the LED array.
    @param colorData The color for the specific pixel. Must formatted as 0x00RRGGBB
  */
    void setPixelRGB(uint32_t index, uint32_t colorData);

    /**
    @brief Copy the LED pixel value of oldIndex into the new index.
    This is useful for shifting frames on the display.

    @param oldIndex Index of value to be copied.
    @param newIndex Location of the copied value.
  */
    void copyIndex(uint32_t oldIndex, uint32_t newIndex);
};
#endif