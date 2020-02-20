/*    The following code has been written to control WS2812 Leds. This library is intended for use with 8x8 matrices, but may be used
 *    with the leds in general. This was written using PlatformIO using the arduino framework for ESP32.
 *
 *    Credit to JSchaenzle: https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT - This wrapper class uses the RMT code found in the link.
 * */

#ifndef EESP32CTRLLED_H
#define EESP32CTRLLED_H

#include "driver/rmt.h"
#include "Arduino.h"

#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0
#define LED_RMT_TX_GPIO GPIO_NUM_26
#define BITS_PER_LED_CMD 24

#define T0H 16  // 0 bit high ticks at  40MHZ  ~ about 400 ns
#define T0L 36  // 0 bit low  ticks at  40MHZ  ~ about 900 ns
#define T1H 36  // 1 bit high ticks at  40MHZ  ~ about 900 ns
#define T1L 24  // 1 bit low  ticks at  40MHZ  ~ about 600 ns

class Esp32CtrlLed {
public:

  rmt_item32_t *LedData;
  uint32_t NUM_LEDS;
  size_t LED_BUFFER_SIZE;
  gpio_num_t LED_CTRL_PIN;
  uint8_t Brightness;

  Esp32CtrlLed(void);
  Esp32CtrlLed(uint16_t NUM_LEDS, gpio_num_t pin);
  Esp32CtrlLed(uint16_t NUM_LEDS, gpio_num_t pin, uint8_t brightness);
  ~Esp32CtrlLed();

  // Set specific pin
  void setPin(gpio_num_t pin);
  void setBrightness(uint8_t brightness);
  // Change the length of the led array
  void updateLength(uint32_t length);

  uint8_t getNthBit(uint32_t number, uint8_t bit);

  // Setup the hardware peripheral. Only call this once.
  void ESP32_RMT_Init(void);

  // Write the Led Data to the
  void write_leds();

  // Clear the WS2812 LEDs
  void resetLeds();

  // Set pixel via separate r, g, b values
  void setPixelRGB(uint32_t index, uint8_t r, uint8_t g, uint8_t b);

  // Set pixal via 32 bit integer
  void setPixelRGB(uint32_t index, uint32_t colorData);

};
#endif