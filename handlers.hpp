#ifndef HANDLERS_CPP
#define HANDLERS_CPP

#include "opcode.hpp"
void interpret(instruction_info info, uint8_t *data);
void interrupt_20(uint8_t *data);
void mov(uint8_t *data);

#endif // !HANDLERS_CPP
