#ifndef _TRAIN_SERVER_TYPES_H_
#define _TRAIN_SERVER_TYPES_H_
#include <track/track_node.h>
typedef struct {
    uint16_t average_velocity;
    uint16_t average_velocity_count;
    track_node* from;
} avg_velocity_t;

#endif // _TRAIN_SERVER_TYPES_H_
