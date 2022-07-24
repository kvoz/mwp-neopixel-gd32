#include "led_fsm.h"

/* Dependencies:
 * - led.h - using public interface of library for setting leds;
 * - timers.h - using software timers for generating non blocking delays;
 * - queue.h - using for checkout own queue for external messages.
 *
 *  */
#include "led.h"
#include "timers.h"
#include "queue.h"

/* Define of FSM states. */
typedef enum {
    LED_FSM_STATE_IDLE,
    LED_FSM_STATE_HOLD,

    LED_FSM_STATE_NUM
} LedFsmStates;

/* Define of FSM events. */
typedef enum {
    LED_FSM_EVENT_NONE,
    LED_FSM_EVENT_HOLD,

    LED_FSM_EVENT_NUM
} LedFsmEvents;

typedef struct Notificator_t {
  uint16_t hold_time;

  // FSM behavior struct.
  struct {
    void (*transitions[LED_FSM_STATE_NUM][LED_FSM_EVENT_NUM])();
    LedFsmStates state;
    LedFsmEvents event;
  } fsm;

} LedpanelInstance;

/* Object-handler of ledpanel system behavior. */
LedpanelInstance lp;

// Queue instances.
#define LED_FSM_QUEUE_RECNUM    (10)
#define LED_FSM_QUEUE_RECSIZE   sizeof(LedFsmQueueRec)   // in bytes
uint8_t queue[LED_FSM_QUEUE_RECNUM * LED_FSM_QUEUE_RECSIZE] = {0};

void _err(void) {
  // Handle error.
  while(1);
}

void idle(void) {
  lp.fsm.state = LED_FSM_STATE_IDLE;

  // Check if there is external events in event queue.
   LedFsmQueueRec rec;
   if (queue_get(QUEUE_LED_FSM, &rec) == 1) {

     switch (rec.msg) {
       case LED_FSM_MSG_SEG1_SHOW1:
         led_show(SEGMENT_1, SEGMENT1_PAT1);

         if (rec.value.hold_time > 0) {
           lp.hold_time = rec.value.hold_time;
           swtimer_reset(TIM_LED_FSM);
           lp.fsm.event = LED_FSM_EVENT_HOLD;
         } else {
           lp.fsm.event = LED_FSM_EVENT_NONE;
         }
         break;

       case LED_FSM_MSG_SEG1_SHOW2:
         led_show(SEGMENT_1, SEGMENT1_PAT2);

         if (rec.value.hold_time > 0) {
           lp.hold_time = rec.value.hold_time;
           swtimer_reset(TIM_LED_FSM);
           lp.fsm.event = LED_FSM_EVENT_HOLD;
         } else {
           lp.fsm.event = LED_FSM_EVENT_NONE;
         }
         break;

       case LED_FSM_MSG_SEG1_SHOW3:
         led_show(SEGMENT_1, SEGMENT1_PAT3);

         if (rec.value.hold_time > 0) {
           lp.hold_time = rec.value.hold_time;
           swtimer_reset(TIM_LED_FSM);
           lp.fsm.event = LED_FSM_EVENT_HOLD;
         } else {
           lp.fsm.event = LED_FSM_EVENT_NONE;
         }
         break;

       default:
         lp.fsm.event = LED_FSM_EVENT_NONE;
         break;
     }
   } else {
     lp.fsm.event = LED_FSM_EVENT_NONE;
   }
}

void hold_show(void) {
  lp.fsm.state = LED_FSM_STATE_HOLD;

  if (swtimer_get(TIM_LED_FSM) >= lp.hold_time) {
    swtimer_stop(TIM_LED_FSM);
    led_clear_all();
    lp.fsm.event = LED_FSM_EVENT_NONE;
  } else {
    lp.fsm.event = LED_FSM_EVENT_HOLD;
  }
}

void led_fsm_reg_callbacks(const void * callbacks) {
  led_reg_callbacks(callbacks);
}

void led_fsm_init(void) {
  /* Perform clear all leds. */
  led_clear_all();
  /* Setup brightness at 50%. */
  led_brightness(50);

  /* Init queue external events. */
  queue_init(QUEUE_LED_FSM,
             LED_FSM_QUEUE_RECNUM,
             LED_FSM_QUEUE_RECSIZE,
             queue);

  lp.fsm.state = LED_FSM_STATE_IDLE;
  lp.fsm.event = LED_FSM_EVENT_NONE;

  lp.fsm.transitions[LED_FSM_STATE_IDLE][LED_FSM_EVENT_NONE] = idle;
  lp.fsm.transitions[LED_FSM_STATE_IDLE][LED_FSM_EVENT_HOLD] = hold_show;

  lp.fsm.transitions[LED_FSM_STATE_HOLD][LED_FSM_EVENT_NONE] = idle;
  lp.fsm.transitions[LED_FSM_STATE_HOLD][LED_FSM_EVENT_HOLD] = hold_show;

}

void led_fsm_call(void) {
  lp.fsm.transitions[lp.fsm.state][lp.fsm.event]();
}
