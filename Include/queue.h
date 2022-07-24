#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>

// Define queue names (ID).
typedef enum {
  QUEUE_LED_FSM,

  QUEUE_NUM
} QueueId;

void queue_init(const QueueId q_id, uint8_t rec_num, uint8_t rec_size, void * q_ptr);
void queue_flush(const QueueId q_id);
void queue_put(const QueueId q_id, const void * element);
uint8_t queue_get(const QueueId q_id, void * element);

#endif /* QUEUE_H_ */
