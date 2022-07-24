#include "led.h"
#include "led_patterns.h"

#include "ws2812.h"

typedef struct {
  SegmentId id;
  ws2812StripId strip;
  uint8_t pattern_num;
  const Pattern * patterns;
} LedSegment;

/* Link segment with its strip line of ws2812b. */
const LedSegment segments[SEGMENT_NUM] = {
    [SEGMENT_1] = {
        .id = SEGMENT_1,
        .strip = WS2812_STRIP_1,
        .pattern_num = SEGMENT1_PAT_NUM,
        .patterns = segment1_patterns,
    },
};

/* Function for register callback to ws2812 led. */
void led_reg_callbacks(const void * callbacks) {
  ws2812_reg_callback(callbacks);
}

/* Function to set patterns for any segment of led panel. */
static void _show(const SegmentId id, const uint8_t pattern_id) {
  if (pattern_id < segments[id].pattern_num) {
    const Pattern * pat = &segments[id].patterns[pattern_id];

    /* Fill buffer with selected pattern. */
    for (uint16_t b = 0; b < pat->block_num; b++) {
      ws2812_color_set_range(segments[id].strip,
                             pat->blocks[b].begin,
                             pat->blocks[b].num,
                             pat->blocks[b].color);
    }

    /* Call function to transmit data to leds. */
    ws2812_update(segments[id].strip);
  }
}

/* Function to show some pattern on selected segment. */
void led_show(const SegmentId seg_id, const uint8_t pattern_id) {
  _show(seg_id, pattern_id);
}

/* Function to setup brightness. */
void led_brightness(const uint8_t brightness) {
  ws2812_brightness_set(brightness);
}

/* Function to clear all segments. */
void led_clear_all(void) {
  /* Reset all data in strip. */
  for (uint8_t id = 0; id < SEGMENT_NUM; id++){
    ws2812_clear(segments[id].strip);
    ws2812_update(segments[id].strip);
  }
}
