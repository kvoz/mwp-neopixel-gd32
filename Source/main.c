#include "main.h"

/* Uses GD32F10x library v2.2.2 */

#include "gd32f10x.h"

#include "systick.h"
#include "led_fsm.h"

#include "timers.h"
#include "queue.h"

#include <time.h>
#include <stdlib.h>

void system_swtimers_init(void);
void init_leds(void);

ws2812_callbacks ws2812_cb = {
    .ws2812_tx_strip1 = strip1_tx_callback,
};

int main(void) {

  dbg_periph_disable(DBG_TIMER0_HOLD);

  systick_config();
  system_swtimers_init();

  nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

  init_leds();
  led_fsm_reg_callbacks(&ws2812_cb);
  led_fsm_init();

  srand(time(NULL));

  LedFsmQueueRec rec;
  rec.msg = LED_FSM_MSG_SEG1_SHOW1;
  rec.value.hold_time = 1000;
  queue_put(QUEUE_LED_FSM, &rec);

  rec.msg = LED_FSM_MSG_SEG1_SHOW2;
  rec.value.hold_time = 1000;
  queue_put(QUEUE_LED_FSM, &rec);

  rec.msg = LED_FSM_MSG_SEG1_SHOW3;
  rec.value.hold_time = 1000;
  queue_put(QUEUE_LED_FSM, &rec);

  while (1) {
    led_fsm_call();
  }
}

void init_leds(void) {
  /* enable TIMERs */
  rcu_periph_clock_enable(RCU_TIMER0);
  timer_deinit(TIMER0);

  /* enable DMAs clock */
  rcu_periph_clock_enable(RCU_DMA0);

  /* enable GPIOs clock */
  rcu_periph_clock_enable(RCU_AF);
  rcu_periph_clock_enable(RCU_GPIOA);

  /* GPIO config */
  // TIM0_CH3
  gpio_init(GPIOA, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

  /* -----------------------------------------------------------------------
   TIMER0_CH3 configurations:
   TIMERxCLK = SystemCoreClock = 108 MHz
   PWM signal via DMA (period = 1.2 us)
   ----------------------------------------------------------------------- */
  timer_oc_parameter_struct timer_ocintpara;
  timer_parameter_struct timer_initpara;

  /* TIMERs configuration */
  timer_initpara.prescaler = 0;
  timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
  timer_initpara.counterdirection = TIMER_COUNTER_UP;
  timer_initpara.period = 135-1;
  timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
  timer_initpara.repetitioncounter = 0;
  timer_init(TIMER0, &timer_initpara);

  /* channel polarity configuration */
  timer_ocintpara.outputstate  = TIMER_CCX_DISABLE;
  timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
  timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
  timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
  timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
  timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
  timer_channel_output_config(TIMER0, TIMER_CH_3, &timer_ocintpara);

  /* channel configuration in PWM mode0*/
  timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_3, 0);
  timer_channel_output_mode_config(TIMER0, TIMER_CH_3, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(TIMER0, TIMER_CH_3,
  TIMER_OC_SHADOW_ENABLE);
  timer_primary_output_config(TIMER0, ENABLE);

  /* send DMA requests at TIM_update event */
  timer_channel_dma_request_source_select(TIMER0, TIMER_DMAREQUEST_UPDATEEVENT);
  /* TIMERs update DMA request for capture compare on channel X */
  timer_dma_enable(TIMER0, TIMER_DMA_CH3D);

  /* shadow register for auto-reload preload enable */
  timer_auto_reload_shadow_enable(TIMER0);
  /* auto-reload preload enable */
  timer_enable(TIMER0);

  /* -----------------------------------------------------------------------
   DMA configuration:
   - DMA0_CH5 <= IMER0_CH3 request;
   memory to peripheral, normal mode, interrupts for full transfer complete
   ----------------------------------------------------------------------- */

  dma_parameter_struct dma_init_struct;

  /* initialize DMA channel */
  dma_deinit(DMA0, DMA_CH3);
  /* DMAx_CHx initialize */
  dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
  dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
  dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
  dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
  dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
  // TIM0_CH3
  dma_init(DMA0, DMA_CH3, &dma_init_struct);

  /* update addresses of peripheral and memory */
  dma_periph_address_config(DMA0, DMA_CH3, (uint32_t)&TIMER_CH3CV(TIMER0));


  /* DMAx_CHx mode configuration */
  dma_circulation_disable(DMA0, DMA_CH3);
  dma_memory_to_memory_disable(DMA0, DMA_CH3);


  /* enable DMAx_CHx transfer complete interrupt */
  dma_interrupt_enable(DMA0, DMA_CH3, DMA_INT_FTF);

  /* enable NVIC for DMAx_CHx IRQ */
  nvic_irq_enable(DMA0_Channel3_IRQn, 0, 0);

  /* Toggle small blocking delay for correct startup of diodes */
  systick_delay_1ms(5);

}

void system_swtimers_init(void) {
  /* ----------------------------------------------------------------------------
     TIMER3 Configuration:
     TIMER3CLK = SystemCoreClock/108 = 1MHz, the period is 1ms.
     ---------------------------------------------------------------------------- */
  timer_parameter_struct timer_initpara;

  rcu_periph_clock_enable(RCU_TIMER3);

  timer_deinit(TIMER3);
  /* initialize TIMER init parameter struct */
  timer_struct_para_init(&timer_initpara);
  /* TIMER1 configuration */
  timer_initpara.prescaler         = 107;
  timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;
  timer_initpara.period            = 999;
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
  timer_init(TIMER3, &timer_initpara);

  timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
  timer_interrupt_enable(TIMER3, TIMER_INT_UP);
  timer_enable(TIMER3);

  nvic_irq_enable(TIMER3_IRQn, 0, 0);
}

void strip1_tx_callback(uint8_t * buffer, uint16_t buff_size) {
  /* Wait if DMA still working. */
  while ( (DMA_CHCTL(DMA0, DMA_CH3) & DMA_CHXCTL_CHEN) == DMA_CHXCTL_CHEN);
  /* Setup address of buffer for DMA. */
  dma_memory_address_config(DMA0, DMA_CH3, (uint32_t) buffer);
  /* Update size of transmission. */
  dma_transfer_number_config(DMA0, DMA_CH3, buff_size);
  /* Enable DMA transfer. */
  dma_channel_enable(DMA0, DMA_CH3);

  /* Enable timer output channel. */
  timer_channel_output_state_config(TIMER0, TIMER_CH_3, TIMER_CCX_ENABLE);
}

// Callback functions from interrupt vectors
void DMA0_Channel3_Callback(void) {
  /* Disable timer channel output. */
  timer_channel_output_state_config(TIMER0, TIMER_CH_3, TIMER_CCX_DISABLE);
  /* disable DMA transfer */
  dma_channel_disable(DMA0, DMA_CH3);
  while ( (DMA_CHCTL(DMA0, DMA_CH3) & DMA_CHXCTL_CHEN) == DMA_CHXCTL_CHEN);
}

void TIMER3_Callback() {
  __disable_irq();
  {
    swtimer_process();
  }
  __enable_irq();
}
