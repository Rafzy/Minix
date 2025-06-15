#include "cpu.hpp"

void exec_parsed(cpu_state *cpu, instruction_info info) {
  if (info.mnemonic == "MOV") {
    exec_mov(cpu, info.op1, info.op2);
  }
  if (info.mnemonic == "INT") {
    exec_int(cpu, info.op1);
  }
};

void init_cpu(cpu_state *cpu) {
  for (int i = 0; i < 8; i++) {
    cpu->registers[i] = 0;
  }
  cpu->registers[SP] = 0x1000;
}

uint16_t get_reg16(cpu_state *cpu, uint8_t reg) {
  if (reg > 7)
    return 0;
  return cpu->registers[reg];
}

void set_reg16(cpu_state *cpu, uint8_t reg, uint16_t val) {
  if (reg > 7)
    return;
  cpu->registers[reg] = val;
}

void exec_mov(cpu_state *cpu, string op1, string op2);
void exec_int(cpu_state *cpu, string op1);
