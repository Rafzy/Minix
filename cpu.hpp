#ifndef CPU_HPP
#define CPU_HPP

#include "opcode.hpp"
#include <cstdint>
#include <unordered_map>

typedef enum { AX = 0, BX, CX, DX, SP, BP, SI, DI } reg_names;

typedef struct {
  uint16_t registers[8];
  std::unordered_map<uint32_t, uint8_t> memory;
} cpu_state;

void init_cpu(cpu_state *cpu);
uint16_t get_reg16(cpu_state *cpu, uint8_t reg);
void set_reg16(cpu_state *cpu, uint8_t reg, uint16_t val);
void exec_parsed(cpu_state *cpu, instruction_info info);
void exec_mov(cpu_state *cpu, string op1, string op2);
void exec_int(cpu_state *cpu, string op1);

#endif // !CPU_HPP
