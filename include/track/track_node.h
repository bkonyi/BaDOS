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
  bool touched; // used for recursing over the paths
  int dist;             /* in millimetres */
};

struct track_node {
  const char *name;
  node_type type;
  int num;              /* sensor or switch number */
  track_node *reverse;  /* same location, but opposite direction */
  track_edge edge[2];
  int state;
  int reserved_by;
  int16_t index;
  track_node* next_reserved;
};

typedef struct track_node_data_t{
  track_node* node;
  int         offset;
} track_node_data_t;

track_node* get_next_track_node(track_node* start_node);
track_node* get_next_track_node_in_path(track_node** start_node);
void set_track_node_state(volatile track_node* node, uint32_t state);
track_node* get_next_sensor( track_node* node);
track_node* get_next_sensor_switch_broken( track_node* node);
bool is_valid_switch_number(uint32_t sw_num) ;
uint32_t get_track_node_length(track_node* node);
uint32_t get_track_node_length_assuming_switch(track_node* node) ;
uint32_t distance_between_track_nodes(track_node* start, track_node * end, bool switch_broken);
uint32_t dist_between_node_and_num(track_node* start, int num);
uint32_t dist_between_node_and_index_using_path( track_node** path_start, int num);
int get_sensor_before_distance(track_node* start_sensor, int distance);
int get_sensor_before_distance_using_path(track_node** start, int distance);
track_node* get_next_sensor_or_exit( track_node* node);
track_node* get_next_sensor_or_exit_using_path(track_node** node) ;
uint32_t distance_between_track_nodes_using_path(track_node** start, track_node * end);
track_node**get_path_iterator(track_node** path,track_node* node) ;
/**
 * @brief Pass it any sensor track node and it  will find the other node based on their position relative to each other
 */
track_node* get_sensor_node_from_num(track_node* start, int num) ;
track_node* track_node_flip(track_node* node);

/**
 * @brief Touch an edge in both directions
 */
void track_touch_edge(track_edge* edge, bool val);
void track_touch_node(track_node* node, bool val);
track_node_data_t track_get_node_location(track_node* last_sensor,int offset);
void track_flip_node_data(track_node_data_t* node_data);
bool _is_track_node_adacent(track_node* node1, track_node* node2);
track_node* get_next_track_node_assuming_switch(track_node* node);
#endif //_TRACK_NODE_H_
