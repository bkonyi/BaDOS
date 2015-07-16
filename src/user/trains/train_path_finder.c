#include <trains/train_path_finder.h>
#include <common.h>
#include <priority_heap.h>
#include <io/io.h>
#include <syscalls.h>

#define MAX_PATH_LENGTH TRACK_MAX

static void calculate_weights(priority_heap_t* min_heap, priority_heap_node_t* previous, track_edge* edge, int16_t* previous_nodes) {
    //If the distance of the edge is 0, the edge doesn't exist
    if(edge->dist != 0) {
        priority_heap_node_t destination_node = keyOf(min_heap, edge->dest->index);

        int temp_dist = previous->value + edge->dist;

        if(temp_dist < destination_node.value) {
            destination_node.value = temp_dist;
            previous_nodes[destination_node.track_node->index] = previous->track_node->index;
            decreaseKey(min_heap, destination_node.track_node->index, destination_node);
        }
    }
}

void find_path(track_node* source, track_node* destination, track_node** path, int* length) {

    ASSERT(source != NULL);
    ASSERT(destination != NULL);
    ASSERT(path != NULL);
    ASSERT(length != NULL);

    int i;
    for(i = 0; i < MAX_PATH_LENGTH; ++i) {
        path[i] = NULL;
    }

    *length = -1;

    int16_t prev[TRACK_MAX];
    for(i = 0; i < TRACK_MAX; ++i) {
        prev[i] = -1;
    }

    //This returns the beginning of the track node array
    track_node* track_start = get_sensor_node_from_num(source, 0);

    priority_heap_t min_heap;
    priority_heap_init(&min_heap);

    //Populate the min heap
    for(i = 0; i < TRACK_MAX; ++i) {
        track_node* temp_node = &track_start[i];

        priority_heap_node_t heap_node;
        heap_node.track_node = temp_node;

        if(temp_node == source) {
            heap_node.value = 0;
        } else {
            heap_node.value = ~0; //Set to UINT_MAX to represent infinity
        }

        insert(&min_heap, temp_node->index, heap_node);
    }

    while(!isEmpty(&min_heap)) {
        priority_heap_node_t min_node_full = minKey(&min_heap);
        deleteMin(&min_heap);
        track_node* min_node = min_node_full.track_node;

        track_edge* straight_edge = &min_node->edge[DIR_AHEAD];
        track_edge* curved_edge = &min_node->edge[DIR_CURVED];
        track_edge* reverse_straight_edge = &min_node->reverse->edge[DIR_AHEAD];
        track_edge* reverse_curved_edge = &min_node->reverse->edge[DIR_CURVED];

        //Update all the weights for the neighbor nodes
        calculate_weights(&min_heap, &min_node_full, straight_edge, prev);
        calculate_weights(&min_heap, &min_node_full, curved_edge, prev);
        calculate_weights(&min_heap, &min_node_full, reverse_straight_edge, prev);
        calculate_weights(&min_heap, &min_node_full, reverse_curved_edge, prev);
    }

    int16_t path_indexes[TRACK_MAX];
    int16_t current_node = destination->index;

    *length = 0;

    while(prev[current_node] != -1) {
        path_indexes[(TRACK_MAX - 1) - *length] = current_node;
        current_node = prev[current_node];
        ++(*length);
    }

    path_indexes[(TRACK_MAX - 1) - *length] = current_node;
    ++(*length);

    int j;
    for(j = 0; j < *length; ++j) {
        path[j] = get_sensor_node_from_num(source, path_indexes[(TRACK_MAX) - *length + j]);
    }
}
