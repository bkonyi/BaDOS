#ifndef _TRACK_NODE_H_
#define _TRACK_NODE_H_
#include <common.h>
typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
  track_edge *reverse;
  track_node *src, *dest;
  int dist;             /* in millimetres */
};

struct track_node {
  const char *name;
  node_type type;
  int num;              /* sensor or switch number */
  track_node *reverse;  /* same location, but opposite direction */
  track_edge edge[2];
  int state;
};

track_node* get_next_track_node(track_node* start_node);
void set_track_node_state(track_node* node, uint32_t state);
track_node* get_next_sensor( track_node* node);
bool is_valid_switch_number(uint32_t sw_num) ;
uint32_t get_track_node_length(track_node* node);
uint32_t distance_between_track_nodes(track_node* start, track_node * end);
#endif //_TRACK_NODE_H_
