#include <track_node.h>

track_node* get_next_track_node(track_node* node) {

	//We can add conditions for getting the next node here
	// like if we only ever want to allow passing over a merge
	// if the switch is set correctly
	return node->edge[node->state].dest;
}
bool is_valid_switch_number(uint32_t sw_num) {
	return (1<=sw_num && sw_num <=32)
            ||(0x99 <= sw_num && sw_num <=0x9c)
            ||(146 <= sw_num && sw_num <=148);
}
void set_track_node_state(track_node* node, uint32_t state) {
	ASSERT(state == DIR_AHEAD 
		|| state == DIR_STRAIGHT 
		|| state ==  DIR_CURVED);
	ASSERT(node != NULL);
	//The only nodes where the state matters are BRANCH
		//Only ever modify those
	if(node->type == NODE_BRANCH) {
		node->state = state;
	} else if(node->type == NODE_MERGE) {
		node->reverse->state = state;
	}
	
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
