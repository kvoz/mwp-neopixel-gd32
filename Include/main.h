#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

/* WS2812 low level callback functions. */
#include "ws2812_config.h"
void strip1_tx_callback(uint8_t * buffer, uint16_t buff_size);

#endif /* MAIN_H */
