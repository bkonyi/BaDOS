#include <track_node.h>

track_node* get_next_track_node(track_node* node) {
	track_node* next = node->edge[node->state].dest;
	//We can add conditions for getting the next node here
	// like if we only ever want to allow passing over a merge
	// if the switch is set correctly
	return next;
}

void set_track_node_state(track_node* node, uint32_t state) {
	KASSERT(state == DIR_AHEAD 
		|| state == DIR_STRAIGHT 
		|| state ==  DIR_CURVED);
	ASSERT(node != NULL);

	node->state = state;
	node->reverse->state = state;
}
track_node* get_next_sensor( track_node* node) {
	track_node* iterator_node;

	if(node == NULL) return NULL;

	for(iterator_node = node->edge[node->state].dest ;iterator_node != NULL ;
		iterator_node = iterator_node->edge[iterator_node->state].dest) {
		if(iterator_node == node) { 
			//We have a cycle and didn't find a sensor
			return NULL;
		}else if(iterator_node->type == NODE_SENSOR) {
			return iterator_node;
		}
	}
	return NULL;
}
