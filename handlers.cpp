#include "handlers.hpp"
#include "opcode.hpp"

void interpret(instruction_info info) {
  if (info.mnemonic == "INT") {
    interrupt_20();
  }
}

void interrupt_20() {}
