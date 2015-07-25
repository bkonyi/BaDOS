#ifndef __PATH_INSTRUCTIONS_H__
#define __PATH_INSTRUCTIONS_H__

#include <track/track_node.h>

#define MAX_PATH_INSTRUCTIONS 100 //Arbitrary again

typedef enum {
    INVALID   = 0,
    STOP      = 1,
    BACK_STOP = 2,
    SWITCH    = 3,
    REVERSE   = 4,
    DONE      = 5
} path_instructions_command_t;

typedef struct {
    path_instructions_command_t command;

    //Stops
    track_node_data_t instruction_node;
    
    //Switches
    int16_t switch_num;
    int16_t direction;
} path_instruction_t;

typedef struct {
    path_instruction_t instructions[MAX_PATH_INSTRUCTIONS];
    int16_t count;
    int16_t front;
} path_instructions_t;

void path_instructions_clear(path_instructions_t* path_instructions);
void path_instructions_insert(path_instructions_t* path_instructions, path_instruction_t instruction);
path_instruction_t path_instruction_peek(path_instructions_t* path_instructions);
path_instruction_t path_instruction_pop(path_instructions_t* path_instructions);

void path_instructions_add_stop(path_instructions_t* path_instructions, track_node* destination, int16_t offset);
void path_instructions_add_back_stop(path_instructions_t* path_instructions, track_node* destination, int16_t offset);
void path_instructions_add_switch(path_instructions_t* path_instructions, int16_t switch_num, int16_t direction);
void path_instructions_add_reverse(path_instructions_t* path_instructions);
void path_instructions_add_done(path_instructions_t* path_instructions);


#endif //__PATH_INSTRUCTIONS_H__
