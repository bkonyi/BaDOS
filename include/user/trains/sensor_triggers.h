#ifndef _SENSOR_TRIGGERS_H
#define _SENSOR_TRIGGERS_H_
#include <common.h>
#include <queue.h> 

#define SENSOR_TRIGGER_MAX_TRIGGERS 80
#define SENSOR_TRIGGER_NUM_SENSORS 80

typedef enum sensor_trigger_type_t {
    TRIGGER_NONE=1,
    TRIGGER_STOP_AT ,
    TRIGGER_STOP_AROUND
}sensor_trigger_type_t;

typedef struct sensor_trigger_info_t {
    sensor_trigger_type_t type;
    int32_t num1;
    int8_t byte1;
    struct sensor_trigger_info_t* next; // Used for queueing
}sensor_trigger_info_t;

CREATE_QUEUE_TYPE(sensor_trigger_info_q,sensor_trigger_info_t);

typedef struct sensor_triggers_t {
    int8_t  sensors[10];
    sensor_trigger_info_q actions[SENSOR_TRIGGER_NUM_SENSORS]; //each sensor gets a command
    sensor_trigger_info_q * free_slots;
}sensor_triggers_t;



void sensor_triggers_init(sensor_triggers_t* triggers, sensor_trigger_info_t * free_slots , sensor_trigger_info_q* free_slots_queue);
void sensor_trigger_unset(sensor_triggers_t* triggers, int16_t sensor_group,int16_t sensor_index);

#define SENSOR_TRIGGER_INFO_INIT(VAR_NAME) \
		sensor_trigger_info_t 	VAR_NAME##_slots[SENSOR_TRIGGER_MAX_TRIGGERS];\
		sensor_triggers_t 		VAR_NAME;\
        sensor_trigger_info_q VAR_NAME##_free_slots_queue;\
		sensor_triggers_init(&VAR_NAME,VAR_NAME##_slots ,&VAR_NAME##_free_slots_queue );



void sensor_triggers_set(sensor_triggers_t *sensor_triggers,int32_t SENSOR_NUM,sensor_trigger_type_t trigger_type,uint8_t* byte,int32_t* num);

sensor_trigger_info_t* sensor_triggers_get(sensor_triggers_t *sensor_triggers, int32_t sensor_num);
void sensor_triggers_add_free_slot(sensor_triggers_t *sensor_triggers,sensor_trigger_info_t* slot);
bool sensor_triggers_has_triggers(sensor_triggers_t *sensor_triggers, int32_t sensor_num);


#endif // _SENSOR_TRIGGERS_H_2
