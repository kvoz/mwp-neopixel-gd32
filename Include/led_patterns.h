#ifndef INCLUDE_LED_PATTERNS_H_
#define INCLUDE_LED_PATTERNS_H_

#include "ws2812.h"

/* Structures for handling patterns:
 * - PatternBlock - minimal piece of pattern;
 * - Pattern - holder for all patterns of led panel.
 *  */
typedef struct {
  uint16_t begin;
  uint16_t num;
  ws2812ColorPalette color;
} PatternBlock;

typedef struct {
  const PatternBlock * blocks;
  uint8_t block_num;
} Pattern;

/* Patterns description for SEGMENT1. */
const PatternBlock segment1_pat1[] = {
    [0] = {.begin = 0, .num = 10, .color = WS2812_COLOR_RED},
};

const PatternBlock segment1_pat2[] = {
    [0] = {.begin = 0, .num = 10, .color = WS2812_COLOR_GREEN},
};

const PatternBlock segment1_pat3[] = {
    [0] = {.begin = 0, .num = 10, .color = WS2812_COLOR_BLUE},
};

const Pattern segment1_patterns[SEGMENT1_PAT_NUM] = {
    [SEGMENT1_PAT1] = { .blocks = segment1_pat1, .block_num = 1 },
    [SEGMENT1_PAT2] = { .blocks = segment1_pat2, .block_num = 1 },
    [SEGMENT1_PAT3] = { .blocks = segment1_pat3, .block_num = 1 },
};

#endif /* INCLUDE_LED_PATTERNS_H_ */
