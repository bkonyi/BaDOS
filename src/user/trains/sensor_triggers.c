#include <trains/sensor_triggers.h>

void _set_trigger_info(sensor_trigger_info_t* trigger_info, sensor_trigger_type_t type, int8_t byte, int32_t num ){
	trigger_info->type = type;
	trigger_info->num1 = num;
	trigger_info->byte1 = byte;
}
void sensor_triggers_init(sensor_triggers_t* triggers, sensor_trigger_info_t * free_slots , sensor_trigger_info_q* free_slots_queue){
    int i;

    QUEUE_INIT(*free_slots_queue);

    for(i = 0; i < 10; ++i) {
        triggers->sensors[i] = 0;
    }
    for(i = 0; i < 80; i ++) {
        QUEUE_INIT(triggers->actions[i]);
    }
    for(i = 0; i < SENSOR_TRIGGER_MAX_TRIGGERS; i ++){
    	QUEUE_PUSH_BACK(*free_slots_queue, free_slots+i);
    }
   	triggers->free_slots = free_slots_queue;
}

//_unset_sensor_trigger
void sensor_trigger_unset(sensor_triggers_t* triggers,int16_t sensor_group,int16_t sensor_index) {
    triggers->sensors[sensor_group] &= ~(1<<(7-sensor_index));
}

void sensor_triggers_set(sensor_triggers_t *sensor_triggers,int32_t sensor_num,sensor_trigger_type_t trigger_type, uint8_t* byte, int32_t* num) {

	uint8_t Byte = 0;
	uint32_t Num = 0;
	if(byte != NULL) Byte = *byte ;
	if(num != NULL) Num = *num ;
	sensor_trigger_info_t* trigger_info;
	QUEUE_POP_FRONT(*(sensor_triggers->free_slots), trigger_info);

	_set_trigger_info(trigger_info, trigger_type, *byte, *num);
	uint32_t sensor_group = (sensor_num) / 8;
    uint32_t sensor_index = (sensor_num) % 8;

    sensor_triggers->sensors[sensor_group] |= 1<<(7-sensor_index);

    QUEUE_PUSH_BACK(sensor_triggers->actions[sensor_num],trigger_info);
   // send_term_heavy_msg(false,"Just set trigger at sensor %d", sensor_num);
}
sensor_trigger_info_t* sensor_triggers_get(sensor_triggers_t *sensor_triggers, int32_t sensor_num){
    sensor_trigger_info_t* sti;
    QUEUE_POP_FRONT(sensor_triggers->actions[sensor_num],sti);
    return sti;
}
void sensor_triggers_add_free_slot(sensor_triggers_t *sensor_triggers,sensor_trigger_info_t* slot) {
	QUEUE_PUSH_BACK(*(sensor_triggers->free_slots),slot);
}
bool sensor_triggers_has_triggers(sensor_triggers_t *sensor_triggers, int32_t sensor_num) {
	return !(IS_QUEUE_EMPTY(sensor_triggers->actions[sensor_num]));
}
