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

uint32_t distance_between_track_nodes_using_path( track_node** path, track_node* start, track_node * end){
	bool start_counting = false;
	if(start == NULL  ) {
		Delay(200);ASSERT(0);
		return 0;
	}else if(end == NULL){
		Delay(200);ASSERT(0);
		return 0;
	}else if(start == end) {
		Delay(200);ASSERT(0);
		return 0;
	}
	track_node* iterator_node; 
	uint32_t dist =0;//get_track_node_length(start);
	uint32_t edge = 0;
	int i ;
	//for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
	//	iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
	for(i =0 ; i < TRACK_MAX; i++) {
		iterator_node = path[i];
		if(!start_counting && iterator_node == start){
			start_counting=true;
		}

		if(start_counting){
			
			if(iterator_node == end){
				//send_term_debug_log_msg("dbtnup got last node %s", end->name);
				break;
			} 
			dist += get_track_node_length(iterator_node);
			edge++;
		}
	}
	if (i == (TRACK_MAX)) {
			Delay(200);ASSERT(0);
			return 0;
		}	
	ASSERT(dist != 0);
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

uint32_t dist_between_node_and_index_using_path( track_node** path, track_node* start, int num){
	bool start_counting = false;
	if(start == NULL  ) {
		Delay(200);ASSERT(0);
		return 0;
	} else if(start->index == num) {
		Delay(200);ASSERT(0);
		return 0;
	}
	track_node* iterator_node; 
	uint32_t dist =0;//get_track_node_length(start);
	uint32_t edge = 0;
	int i ;
	//for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
	//	iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
	for(i =0 ; i < TRACK_MAX; i++) {
		iterator_node = path[i];
		if(!start_counting && iterator_node == start){
			send_term_debug_log_msg("Found start node");
			start_counting=true;
		}

		if(start_counting){
			
			if(iterator_node->index == num){
				//send_term_debug_log_msg("dbtnup got last node %s", end->name);
				break;
			} 
			dist += get_track_node_length(iterator_node);
			edge++;
		}
	}
	if (i == (TRACK_MAX)) {
			Delay(200);ASSERT(0);
			return 0;
	}	

	ASSERT(dist != 0);
	return dist;
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
track_node* get_next_sensor_or_exit_using_path(track_node** path, track_node* node) {
	track_node* iterator_node;

	if(node == NULL || node->type == NODE_EXIT) return NULL;
	int i ;
	bool start_looking = false;
	for(i =0 ; i < TRACK_MAX; i++) {
		iterator_node = path[i];
		//send_term_debug_log_msg("node: %s iterator node: %s",node->name,iterator_node->name);
		if(!start_looking && iterator_node == node){
			start_looking = true;
			//and we want to skip looking at the first one
		}else if (start_looking ){
			if(iterator_node == NULL){
				return NULL;
			} 
			//send_term_debug_log_msg("gnsoeup st on: %s %s",node->name,iterator_node->name);
			if(iterator_node->type == NODE_SENSOR || iterator_node->type == NODE_EXIT) {
				return iterator_node;
			}
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
	ASSERT(start->type == NODE_SENSOR);

	int index_num = start->num;
	track_node* base = start - index_num;
	return base+num;
}


int get_sensor_before_distance(track_node* start_sensor, int distance) {
    ASSERT(start_sensor->type == NODE_SENSOR);
    track_node* iterator_node;
    uint32_t partial_distance = 0;
    uint32_t segment_dist = 0;
    int print_index = 0;

    if(distance < 0) {
    	return -1;
    }

    for(iterator_node = start_sensor; iterator_node != NULL; iterator_node = get_next_sensor_or_exit(iterator_node)) {
            track_node* next_node = get_next_sensor_or_exit(iterator_node);
            segment_dist = distance_between_track_nodes(iterator_node, next_node, false);
            printf(COM2, "\033[s\033[%d;%dHIterator Node Name: %s Segment distance: %d Total Distance: %d Desired Distance: %d\033[u", 40 + print_index++, 60, iterator_node->name, segment_dist, partial_distance + segment_dist, distance);
            partial_distance += segment_dist;

            if(partial_distance >= distance || next_node->type == NODE_EXIT) {
	  		    send_term_heavy_msg(false, "Going to trigger at sensor: %s Current Sensor: %s", iterator_node->name, start_sensor->name);
            	return iterator_node->num;
            }
    }
    return -2;
}
int get_sensor_before_distance_using_path(track_node** path,track_node* start_sensor, int distance) {
    ASSERT(start_sensor->type == NODE_SENSOR);
    track_node* iterator_node;
    uint32_t partial_distance = 0;
    uint32_t segment_dist = 0;
    
    bool start_looking = false;
    if(distance < 0) {
    	return -1;
    }
    int i ;
	//for(iterator_node = get_next_track_node(start) ;iterator_node != end   ;
	//	iterator_node = distance_between_track_nodes_next(iterator_node,&broken_switch)) {
	for(i =0 ; i < TRACK_MAX; i++) {
		iterator_node = path[i];
		if(!start_looking && iterator_node == start_sensor) {
			start_looking = true;

			send_term_debug_log_msg("STARTEDERsddfa looking on name %s", iterator_node->name);
		}
		if(start_looking== true){
			//send_term_debug_log_msg("gsbdup name %s", iterator_node->name);
			if(!(iterator_node->type == NODE_SENSOR  || iterator_node->type == NODE_EXIT)) continue;

			track_node* next_node = get_next_sensor_or_exit_using_path(path,iterator_node);
			if(next_node== NULL){
				Delay(200);
			ASSERT(0);
			}
			//send_term_debug_log_msg("Our Node %s Node we are looking for %s",iterator_node->name,next_node->name);
	        segment_dist = distance_between_track_nodes_using_path(path,iterator_node, next_node);
	        //send_term_debug_log_msg("  NodeName:%s Segdis:%d ToDis:%d Des Dis:%d", iterator_node->name, segment_dist, partial_distance + segment_dist, distance);
	        partial_distance += segment_dist;

	        if(partial_distance >= distance || next_node->type == NODE_EXIT) {
	  		    send_term_heavy_msg(false, "Will trigger at sensor: %s Curr Sensor: %s", iterator_node->name, start_sensor->name);
	        	return iterator_node->num;
	        }	
		}
		
		
	}
    return -2;
}

