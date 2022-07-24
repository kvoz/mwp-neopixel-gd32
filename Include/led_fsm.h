#ifndef INCLUDE_LED_FSM_H_
#define INCLUDE_LED_FSM_H_

#include <stdint.h>

#include "led.h"

/* Queue objects. */
typedef enum {
  LED_FSM_MSG_SEG1_SHOW1,
  LED_FSM_MSG_SEG1_SHOW2,
  LED_FSM_MSG_SEG1_SHOW3,

  LED_FSM_MSG_NUM,
} LedFsmMsg;

typedef struct {
  LedFsmMsg msg;

  struct {
    uint16_t hold_time;
  } value;
} LedFsmQueueRec;

void led_fsm_init(void);
void led_fsm_call(void);
void led_fsm_reg_callbacks(const void * callbacks);

#endif /* INCLUDE_LED_FSM_H_ */
