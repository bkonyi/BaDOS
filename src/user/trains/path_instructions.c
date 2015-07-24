#include <trains/path_instructions.h>
#include <common.h>

void path_instructions_clear(path_instructions_t* path_instructions) {
    memset(&path_instructions, 0, sizeof(path_instructions_t));
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
    path_instruction_t instruction;
    instruction.command = STOP;
    instruction.instruction_node.node = destination;
    instruction.instruction_node.offset = offset;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_back_stop(path_instructions_t* path_instructions, track_node* destination, int16_t offset) {
    path_instruction_t instruction;
    instruction.command = BACK_STOP;
    instruction.instruction_node.node = destination;
    instruction.instruction_node.offset = offset;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_switch(path_instructions_t* path_instructions, int16_t switch_num, int16_t direction) {
    path_instruction_t instruction;
    instruction.command = SWITCH;
    instruction.switch_num = switch_num;
    instruction.direction = direction;

    path_instructions_insert(path_instructions, instruction);
}

void path_instructions_add_reverse(path_instructions_t* path_instructions) {
    path_instruction_t instruction;
    instruction.command = REVERSE;

    path_instructions_insert(path_instructions, instruction);
}

