#include <track_node.h>
#include <io.h>
#include <bwio.h>
#include <terminal/terminal.h>
#include <terminal/terminal_debug_log.h>

track_node* get_next_track_node(track_node* node) {

	//We can add conditions for getting the next node here
	// like if we only ever want to allow passing over a merge
	// if the switch is set correctly
	return node->edge[node->state].dest;
}

track_node* get_next_track_node_in_path(track_node** node) {
	return *(node + 1);
}

bool is_valid_switch_number(uint32_t sw_num) {
	return (1<=sw_num && sw_num <=32)
            ||(0x99 <= sw_num && sw_num <=0x9c)
            ||(146 <= sw_num && sw_num <=148);
}
void set_track_node_state(volatile track_node* node, uint32_t state) {
	ASSERT(state == DIR_AHEAD 
		|| state == DIR_STRAIGHT 
		|| state ==  DIR_CURVED);
	ASSERT(node != NULL);
	//The only nodes where the state matters are BRANCH
		//Only ever modify those
	if(node->type == NODE_BRANCH) {
		node->state = state;
	}
}
uint32_t get_track_node_length(track_node* node) {
	if(node == NULL){
		return 0;
	}else {
		return node->edge[node->state].dist;
	}
}

static track_node* distance_between_track_nodes_next(track_node* node, bool* flip_switches) {
	int index = node->state;
	if(*flip_switches == true && node->type == NODE_BRANCH) {
		*flip_switches = false;
		index = !(*flip_switches);
		index = !(node->state);
	}
	return node->edge[index].dest;
}

uint32_t distance_between_track_nodes(track_node* start, track_node * end, bool broken_switch){
	if(start == NULL || end == NULL || start == end) {
		return 0;
	}
	track_node* iterator_node; 
	uint32_t dist =get_track_node_length(start);
	uint32_t edge = 0;

	for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
		iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
		
		if(iterator_node == start || iterator_node == NULL){
			//We have a cycle and didn't find a sensor
			return 0;
		}
		dist += get_track_node_length(iterator_node);
		edge++;

	}
	ASSERT(dist != 0);
	return dist;
}

uint32_t distance_between_track_nodes_using_path(track_node** start, track_node * end){

	if(start == NULL  ) {
		Delay(200);ASSERT(0);
		return 0;
	}else if(end == NULL){
		Delay(200);ASSERT(0);
		return 0;
	}
	track_node** iterator_node; 
	uint32_t dist =0;//get_track_node_length(start);
	//for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
	//	iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {

	for(iterator_node = start ; iterator_node != NULL && iterator_node[0] != NULL && iterator_node[0] != end; iterator_node++) { 
		//printf(COM2,"INTTER 0x%x end 0x%x\r\n",(uint32_t)iterator_node,(uint32_t)end);
		dist += get_track_node_length(iterator_node[0]);
	}

	if ( iterator_node == NULL ) {
		Delay(200);ASSERT(0);
		return 0;
	}	
	if(dist ==0 && start[0] != end){
		ASSERT(0);	
	}
	
	return dist;
}

uint32_t dist_between_node_and_num(track_node* start, int num) {
	if(start == NULL || start->num == num) {
		return 0;
	}
	bool broken_switch = false;
	track_node* iterator_node; 
	uint32_t dist = get_track_node_length(start);
	uint32_t edge = 0;

	for(iterator_node = get_next_track_node(start) ;iterator_node->num !=num   ;
		iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
		
		if(iterator_node == start || iterator_node == NULL){
			//We have a cycle and didn't find a sensor
			return 0;
		}
		dist += get_track_node_length(iterator_node);
		edge++;

	}
	ASSERT(dist != 0);
	return dist;
}

uint32_t dist_between_node_and_index_using_path( track_node** path_start, int num){
	if(path_start == NULL) {
		Delay(200);ASSERT(0);
		return 0;
	}else if(path_start[0] == NULL  ) {
		Delay(200);ASSERT(0);
		return 0;
	} else if(path_start[0]->index == num) {
		Delay(200);ASSERT(0);
		return 0;
	}
	track_node** iterator_path_node; 
	uint32_t dist =0;//get_track_node_length(start);

	//for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
	//	iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
	for(iterator_path_node = path_start ; iterator_path_node != NULL && iterator_path_node[0] !=NULL; iterator_path_node++) {
		if(iterator_path_node[0]->index == num){
			//send_term_debug_log_msg("dbtnup got last node %s", end->name);
			ASSERT(dist!=0);
			return dist;
		} 
		dist += get_track_node_length(iterator_path_node[0]);	
	}
	ASSERT(0);
	return 0;
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

track_node* get_next_sensor_or_exit( track_node* node) {
	track_node* iterator_node;

	if(node == NULL || node->type == NODE_EXIT) return NULL;

	for(iterator_node = node->edge[node->state].dest ;iterator_node != NULL ;
		iterator_node = iterator_node->edge[iterator_node->state].dest) {
		if(iterator_node == node) { 
			//We have a cycle and didn't find a sensor
			return NULL;
		}else if(iterator_node->type == NODE_SENSOR || iterator_node->type == NODE_EXIT) {
			return iterator_node;
		}
	}
	return NULL;
}

track_node* get_next_sensor_or_exit_using_path( track_node** node) {
	track_node** iterator_node;

	if(node == NULL || node[0] == NULL || node[0]->type == NODE_EXIT) return NULL;

	for(iterator_node = node+1 ; iterator_node != NULL && iterator_node[0] != NULL; iterator_node++) {
		//send_term_debug_log_msg("gnsoeup st on: %s %s",node->name,iterator_node->name);
		if(iterator_node[0]->type == NODE_SENSOR || iterator_node[0]->type == NODE_EXIT) {
			return iterator_node[0];
		}		
	}
	return NULL;
}

track_node* get_next_sensor_switch_broken( track_node* node) {
	track_node* iterator_node;

	if(node == NULL) return NULL;

	int state = node->state;
	bool first_branch = true;

	for(iterator_node = node->edge[state].dest ;iterator_node != NULL ;
		iterator_node = iterator_node->edge[state].dest) {
		if(iterator_node == node) { 
			//We have a cycle and didn't find a sensor
			return NULL;
		}else if(iterator_node->type == NODE_SENSOR) {
			return iterator_node;
		}

		state = iterator_node->state;

		if(iterator_node->type == NODE_BRANCH && first_branch) {
			state = (state == DIR_CURVED) ? DIR_AHEAD : DIR_CURVED;
			first_branch = false;
		}
	}
	return NULL;
}

track_node* get_sensor_node_from_num(track_node* start, int num) {
	//ASSERT(start->type == NODE_SENSOR);

	int index_num = start->index;
	track_node* base = start - index_num;
	return base+num;
}


int get_sensor_before_distance(track_node* start_sensor, int distance) {
    //ASSERT(start_sensor->type == NODE_SENSOR);
    track_node* iterator_node;
    uint32_t partial_distance = 0;
    uint32_t segment_dist = 0;

    if(distance < 0) {
    	return -1;
    }

    for(iterator_node = start_sensor; iterator_node != NULL; iterator_node = get_next_sensor_or_exit(iterator_node)) {
            track_node* next_node = get_next_sensor_or_exit(iterator_node);
            segment_dist = distance_between_track_nodes(iterator_node, next_node, false);
            partial_distance += segment_dist;

            if(partial_distance >= distance || next_node->type == NODE_EXIT) {
	  		    send_term_heavy_msg(false, "Going to trigger at sensor: %s Current Sensor: %s", iterator_node->name, start_sensor->name);
            	return iterator_node->num;
            }
    }
    return -2;
}

int get_sensor_before_distance_using_path(track_node** start, int distance) {
    //ASSERT(start[0]->type == NODE_SENSOR);

    track_node** iterator_node;
    uint32_t partial_distance = 0;
    uint32_t segment_dist = 0;

    if(distance < 0) {
    	return -1;
    }

    //send_term_debug_log_msg("[SENSOR_DIST_PATH] Distance: %d", distance);

	//for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
	//	iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
	for(iterator_node = start ;iterator_node != NULL; iterator_node++) {
		//send_term_debug_log_msg("gsbdup name %s", iterator_node->name);
		//if(!(iterator_node[0]->type == NODE_SENSOR  || iterator_node[0]->type == NODE_EXIT)) continue;

		track_node* next_node = *(iterator_node + 1);//get_next_sensor_or_exit_using_path(iterator_node);
		if(next_node== NULL){
			return -3;
		}

        segment_dist = distance_between_track_nodes_using_path(iterator_node, next_node);
        //send_term_debug_log_msg("Segment DEEST (%s->%s) %d",iterator_node[0]->name, next_node->name, segment_dist);
        partial_distance += segment_dist;
        //send_term_debug_log_msg("[SENSOR_DIST_PATH] Partial Distance: %d", partial_distance);

        if((partial_distance >= distance && iterator_node[0]->type == NODE_SENSOR) || next_node->type == NODE_EXIT) {
        	return iterator_node[0]->num;
        }	
	}
    return -2;
}

track_node**get_path_iterator(track_node** path,track_node* node) {
	while(path[0] != NULL){
		
		if(path[0] == node) {
			return path;
		}
		path++;
	}
	return NULL;
}

track_node* track_node_flip(track_node* node) {
  return (node->edge[node->state].dest->reverse);
}
