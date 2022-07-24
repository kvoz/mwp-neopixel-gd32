#ifndef LED_H_
#define LED_H_

#include "ws2812_config.h"

typedef enum {
 SEGMENT_1,

 SEGMENT_NUM
} SegmentId;

typedef enum {
  SEGMENT1_PAT1,
  SEGMENT1_PAT2,
  SEGMENT1_PAT3,

  SEGMENT1_PAT_NUM
} Segment1Patterns;

void led_reg_callbacks(const void * callbacks);
void led_show(const SegmentId seg_id, const uint8_t pattern_id);
void led_brightness(const uint8_t brightness);
void led_clear_all(void);

#endif /* LED_H_ */
