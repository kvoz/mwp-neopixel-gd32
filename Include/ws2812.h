#ifndef WS2812_H_
#define WS2812_H_

#include <stdint.h>

/* Predefined stripes ID. */
typedef enum {
  WS2812_STRIP_1,

  WS2812_STRIP_NUM
} ws2812StripId;

/* Predefined number of pixels in stripes. */
enum {
  STRIP_1_PIXEL_NUM = 36,
};

typedef enum {
  WS2812_COLOR_OFF,
  WS2812_COLOR_RED,
  WS2812_COLOR_GREEN,
  WS2812_COLOR_BLUE,
  WS2812_COLOR_YELLOW,
  WS2812_COLOR_PURPLE,

  WS2812_COLOR_NUM
} ws2812ColorPalette;

void ws2812_reg_callback(const void * callbacks);

void ws2812_clear(const ws2812StripId strip);
void ws2812_color_set(const ws2812StripId strip,
                      const uint16_t pixel,
                      const ws2812ColorPalette color);
void ws2812_color_set_range(const ws2812StripId strip,
                            const uint16_t pixel_begin,
                            const uint16_t pixel_end,
                            const ws2812ColorPalette color);
ws2812ColorPalette ws2812_color_get(const ws2812StripId strip, const uint16_t pixel);
void ws2812_brightness_set(const uint8_t brightness);
void ws2812_update(const ws2812StripId strip);

#endif /* WS2812_H_ */
