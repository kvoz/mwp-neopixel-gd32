#include "ws2812.h"
#include "ws2812_config.h"

#include <stdint.h>
#include <stddef.h>

/* Only supports capacity for color depth equals 8 bits.
 * So there is some magic numbers below.
*/

/* Guard slots definition. Slots are filled with zero value.
 * 1 slot = 1 period of TIMx (1.25 us), handling ws2812 communications.
 * Start guard slot do reset pulse for communication, stop guard slot corrects
 * align of DMA because of enabled shadow registers TIMx.
 * Stop guard slot must be at least = 1, start guard slot must be > 0
 * and perform reset pulse more than 280 us (WS2812B-v5 datasheet).
 * */
#define WS2812_SLOT_GUARD_START     (240)
#define WS2812_SLOT_GUARD_END       (1)
#define CALC_BUFF_SIZE(pixels)      (WS2812_SLOT_GUARD_START + pixels * 24 + WS2812_SLOT_GUARD_END)

/* Internal buffers for ws2812 stripes.
 *
 * Scaling:
 * 1. Expand ws2812StripId with additional strip name (at ws2812.h).
 * 2. Add number of pixel in strip to enumerate (at ws2812.h).
 * 3. Add new static buffer for ws2812 strip.
 * 4. Register new strip in array leds[] as follows.
 * 5. From upper layer register HW function for sending data to each strip via ws2812_reg_func_tx().
 *
 * */
uint8_t strip_1_buff[CALC_BUFF_SIZE(STRIP_1_PIXEL_NUM)];

typedef struct {
  uint8_t * const buff;
  const uint16_t buff_size;
  const uint16_t pix_num;
  void (* send_data) (uint8_t * buffer, uint16_t buff_size);
} LedStrip;

/* Dummy function for safe callback init. */
static void _tx_callback(uint8_t * buffer, uint16_t buff_size) {
  (void)(buffer);
  (void)(buff_size);
};

LedStrip leds[WS2812_STRIP_NUM] = {
    [WS2812_STRIP_1] = {
        .pix_num = STRIP_1_PIXEL_NUM,
        .buff = strip_1_buff,
        .buff_size = CALC_BUFF_SIZE(STRIP_1_PIXEL_NUM),
        .send_data = _tx_callback,
    },

};

/* Predefined colors. */
typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} ColorRGB;

/* Color limit values in each channel. */
#define COLOR_VALUE_MAX       (255)
#define COLOR_VALUE_MIN       (5)
/* Brightness in percentage. */
#define COLOR_BRIGHTNESS_MAX   (100)
#define COLOR_BRIGHTNESS_MIN   (((float)COLOR_VALUE_MIN / (float)COLOR_VALUE_MAX) * 100)

ColorRGB rgb_palette[WS2812_COLOR_NUM] = {
    [WS2812_COLOR_OFF]    = { 0,                  0,                  0                 },
    [WS2812_COLOR_RED]    = { COLOR_VALUE_MAX,    0,                  0                 },
    [WS2812_COLOR_GREEN]  = { 0,                  COLOR_VALUE_MAX,    0                 },
    [WS2812_COLOR_BLUE]   = { 0,                  0,                  COLOR_VALUE_MAX   },
    [WS2812_COLOR_YELLOW] = { COLOR_VALUE_MAX,    COLOR_VALUE_MAX,    0                 },
    [WS2812_COLOR_PURPLE] = { COLOR_VALUE_MAX,    0,                  COLOR_VALUE_MAX   },
};

/* Defines periods for timer to setup:
 * - BIT_RESET = about 1/3 of period of TIM
 * - BIT_SET = about 2/3 of period of TIM
 * */
#define WS2812_BIT_SET          ((uint8_t)(((WS2812_TIM_FREQ * 2 / (WS2812_DATA_RATE * 3))) - 1))
#define WS2812_BIT_RESET        ((uint8_t)(((WS2812_TIM_FREQ / (WS2812_DATA_RATE * 3))) - 1))

static void _color_set(uint16_t pixel,
                       ColorRGB * const color,
                       uint8_t * buffer) {
  /* Select pixel in array. */
  uint16_t px = WS2812_SLOT_GUARD_START + pixel * 24;

  /* Fill color code combination from color palette for pixel. */
  for (uint8_t i = 0; i < 8; i++) {

    if( ((color->green << i) & 0x80) != 0 ) {
      buffer[px + i] = WS2812_BIT_SET;
    } else {
      buffer[px + i] = WS2812_BIT_RESET;
    }

    if( ((color->red << i) & 0x80) != 0 ) {
      buffer[px + i + 8] = WS2812_BIT_SET;
    } else {
      buffer[px + i + 8] = WS2812_BIT_RESET;
    }

    if( ((color->blue << i) & 0x80) != 0 ) {
      buffer[px + i + 16] = WS2812_BIT_SET;
    } else {
      buffer[px + i + 16] = WS2812_BIT_RESET;
    }
  }
}

static void _reset_set(uint8_t * buffer, uint16_t buffer_size) {
  // Setup reset bits on begin of data frame.
  for (uint16_t i = 0; i < WS2812_SLOT_GUARD_START; i++) {
    buffer[i] = 0;
  }

  // Setup reset bits on end of data frame.
  for (uint16_t i = buffer_size - WS2812_SLOT_GUARD_END; i < buffer_size; i++) {
    buffer[i] = 0;
  }

}

void ws2812_color_set(const ws2812StripId strip,
                      const uint16_t pixel,
                      const ws2812ColorPalette color) {
  _color_set(pixel, &rgb_palette[color], leds[strip].buff);
}

void ws2812_color_set_range(const ws2812StripId strip,
                            const uint16_t pixel_begin,
                            const uint16_t pixel_num,
                            const ws2812ColorPalette color) {
  /* Checkout overflow condition 1. */
  if (pixel_begin >= leds[strip].pix_num) {
    return;
  }
  /* Checkout overflow condition 2. */
  if (pixel_num > leds[strip].pix_num - pixel_begin) {
    return;
  }

  for (uint16_t i = pixel_begin; i < pixel_begin + pixel_num; i++) {
    _color_set(i, &rgb_palette[color], leds[strip].buff);
  }
}

void ws2812_clear(ws2812StripId strip) {
  /* Clear all LEDs in strip. */
  for (uint16_t led = 0; led < leds[strip].pix_num; led++) {
    _color_set(led, WS2812_COLOR_OFF, leds[strip].buff);
  }
}

void ws2812_reg_callback(const void * callbacks) {
  if (callbacks != NULL) {
    leds[WS2812_STRIP_1].send_data = ((ws2812_callbacks *)callbacks)->ws2812_tx_strip1;
  }

  for (uint8_t i = 0; i < WS2812_STRIP_NUM; i++) {
    ws2812_clear(i);
  }
}

ws2812ColorPalette ws2812_color_get(const ws2812StripId strip, const uint16_t pixel) {
  ws2812ColorPalette pixel_color = WS2812_COLOR_OFF;
  ColorRGB color = {0};
  /* Select pixel in array. */
  uint16_t px = WS2812_SLOT_GUARD_START + pixel * 24;
  /* Set color code combination in GRB for pixel. */
  for (uint8_t i = 0; i < 8; i++) {
    if (leds[strip].buff[px + i] == WS2812_BIT_SET) {
      color.green |= 1 << (7 - i);
    } else {
      color.green |= 0 << i;
    }

    if (leds[strip].buff[px + 8 + i] == WS2812_BIT_SET) {
      color.red |= 1 << (7 - i);
    } else {
      color.red |= 0 << i;
    }

    if (leds[strip].buff[px + 16 + i] == WS2812_BIT_SET) {
      color.blue |= 1 << (7 - i);
    } else {
      color.blue |= 0 << i;
    }
  }

  for (uint16_t i = 0; i < WS2812_COLOR_NUM; i++) {
    if (color.red == rgb_palette[i].red &&
        color.green == rgb_palette[i].green &&
        color.blue == rgb_palette[i].blue) {
      pixel_color = i;
      break;
    }
  }

  return pixel_color;
}

void ws2812_brightness_set(const uint8_t brightness) {
  for (uint8_t c = WS2812_COLOR_RED; c < WS2812_COLOR_NUM; c++) {

    if (brightness < COLOR_BRIGHTNESS_MIN) {
      if (rgb_palette[c].green != 0) rgb_palette[c].green = COLOR_VALUE_MIN;
      if (rgb_palette[c].red != 0) rgb_palette[c].red = COLOR_VALUE_MIN;
      if (rgb_palette[c].blue != 0) rgb_palette[c].blue = COLOR_VALUE_MIN;
    }
    else if (brightness < COLOR_BRIGHTNESS_MAX) {

      if (rgb_palette[c].green != 0) {
        rgb_palette[c].green = (uint8_t) ((float)(COLOR_VALUE_MAX / 100) * brightness);
      }

      if (rgb_palette[c].red != 0) {
        rgb_palette[c].red = (uint8_t) ((float)(COLOR_VALUE_MAX / 100) * brightness);
      }

      if (rgb_palette[c].blue != 0) {
        rgb_palette[c].blue = (uint8_t) ((float)(COLOR_VALUE_MAX / 100) * brightness);
      }
    } else {
      if (rgb_palette[c].green != 0) rgb_palette[c].green = COLOR_VALUE_MAX;
      if (rgb_palette[c].red != 0) rgb_palette[c].red = COLOR_VALUE_MAX;
      if (rgb_palette[c].blue != 0) rgb_palette[c].blue = COLOR_VALUE_MAX;
    }

  }
}

void ws2812_update(const ws2812StripId strip) {
  // For safety reset part of data for RST pause on sending data frame.
  _reset_set(leds[strip].buff, leds[strip].buff_size);
  // Call function to transmit data.
  leds[strip].send_data(leds[strip].buff, leds[strip].buff_size);
}
