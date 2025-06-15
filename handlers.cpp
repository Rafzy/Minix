#include "handlers.hpp"
#include "opcode.hpp"
#include "registers.hpp"

void interpret(instruction_info info, uint8_t *data) {
  if (info.mnemonic == "INT") {
    interrupt_20(data);
  }
  if (info.mnemonic == "MOV") {
    mov(data);
  }
}

void interrupt_20(uint8_t *data) {}
void mov(uint8_t *data) {}
