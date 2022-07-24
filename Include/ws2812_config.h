#ifndef INCLUDE_WS2812_CONFIG_H_
#define INCLUDE_WS2812_CONFIG_H_

#include <stdint.h>

/* Definition of timer frequency, which communicates with WS2812 (in Hz). */
#define WS2812_TIM_FREQ         ((uint32_t) 108000000)
/* Definition of communication speed with WS2812 (800 kHz default). */
#define WS2812_DATA_RATE        ((uint32_t) 800000)

typedef struct {
  void (*ws2812_tx_strip1) (uint8_t * buffer, uint16_t buff_size);
} ws2812_callbacks;

#endif /* INCLUDE_WS2812_CONFIG_H_ */
