# ws2812-simple
Minimal working project for handling ws2812b (Neopixel) via simple led driver (based on FSM) for GD32F103.

Project info:
* MCU: GD32F103RET6 (analog STM32F103RET6)
* GD firmware version: 2.2.2 (2021-12-16)

### Features

1. No dynamic allocations, all data full static.
1. Hardware abstracted (example uses TIM+DMA data transfer variation on some ARM MCU).
1. Abstracted library `led.h`: all pixel calculations hided, show only predefined user patterns.
1. Simple usage without `led.h`: through interface of `ws2812.h` .

([back to top](#top))


### Limitations

1. There is some TODOs in `queue.c`.

([back to top](#top))

### Usage

1. In `ws2812_config.h` expand callback structure if need.
1. In `ws2812.h` expand `ws2812StripId` and define number of pixels.
1. In `ws2812.c` make as many `strip_x_buff[]` as you need, then register them in array `leds[]`.
1. In `led.h` correct `SegmentId` and perform pattern enumerate for each segment.
1. Write your patterns in `led_patterns.h` using arrays of `PatternBlock` structure on each pattern.
1. In `led.c` register them in array `segments[]`.
1. In `led_fsm.h` make support for new segment messages for FSM queue.
1. In `led_fsm.c` improve FSM transition table and calling functions for any scenario.

([back to top](#top))