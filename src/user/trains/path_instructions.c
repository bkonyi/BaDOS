#include <trains/path_instructions.h>
#include <terminal/terminal_debug_log.h>
#include <common.h>

void path_instructions_clear(path_instructions_t* path_instructions) {
    memset(path_instructions, 0, sizeof(path_instructions_t));
}

void path_instructions_insert(path_instructions_t* path_instructions, path_instruction_t instruction) {
    path_instructions->instructions[path_instructions->count++] = instruction;
}

path_instruction_t path_instruction_peek(path_instructions_t* path_instructions) {
    if(path_instructions->count == path_instructions->front) {
        path_instruction_t empty;
        memset(&empty, 0, sizeof(path_instruction_t));
        return empty;
    }

    return path_instructions->instructions[path_instructions->front];
}

path_instruction_t path_instruction_pop(path_instructions_t* path_instructions) {
    if(path_instructions->count == path_instructions->front) {
        path_instruction_t empty;
        memset(&empty, 0, sizeof(path_instruction_t));
        return empty;
    }

    return path_instructions->instructions[path_instructions->front++];
}

void path_instructions_add_stop(path_instructions_t* path_instructions, track_node* destination, int16_t offset) {
    send_term_debug_log_msg("[PATH_INST] Adding stop at: %s offset: %d", destination->name, offset);
    path_instruction_t instruction;
    instruction.command = STOP;
    instruction.instruction_node.node = destination;
    instruction.instruction_node.offset = offset;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_back_stop(path_instructions_t* path_instructions, track_node* destination, int16_t offset) {
    send_term_debug_log_msg("[PATH_INST] Adding back stop at: %s offset: %d", destination->name, offset);
    path_instruction_t instruction;
    instruction.command = BACK_STOP;
    instruction.instruction_node.node = destination;
    instruction.instruction_node.offset = offset;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_switch(path_instructions_t* path_instructions, track_node* switch_node, int16_t direction) {
    send_term_debug_log_msg("[PATH_INST] Setting switch: %d direction: %c", switch_node->num, (direction == DIR_STRAIGHT) ? 'S' : 'C'); 
    path_instruction_t instruction;
    instruction.command = SWITCH;
    instruction.instruction_node.node = switch_node;
    instruction.switch_num = switch_node->num;
    instruction.direction = direction;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_reverse(path_instructions_t* path_instructions, track_node* reverse_node) {
    send_term_debug_log_msg("[PATH_INST] Adding reverse");

    path_instruction_t instruction;
    instruction.command = REVERSE;
    instruction.instruction_node.node = reverse_node;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_done(path_instructions_t* path_instructions) {
    send_term_debug_log_msg("[PATH_INST] Adding done");
    path_instruction_t instruction;
    instruction.command = DONE;

    path_instructions_insert(path_instructions, instruction);
}

