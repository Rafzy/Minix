#ifndef CPU_HPP
#define CPU_HPP

#include "opcode.hpp"
#include <cstdint>

#define MEMORY_SIZE (1 << 20);

typedef enum { REGISTER = 0, IMMEDIATE, MEMORY } types;

typedef struct {
  // std::unordered_map<uint32_t, uint8_t> data;
  uint8_t data[(1 << 20)];
} memory_t;

typedef struct {
  uint16_t registers[17];
  memory_t *memory;
} cpu_state_t;

void init_cpu(cpu_state_t *cpu);
uint16_t get_reg16(cpu_state_t *cpu, uint8_t reg);
void set_reg16(cpu_state_t *cpu, uint8_t reg, uint16_t val);
void exec_parsed(cpu_state_t *cpu, instruction_info &info);
void exec_mov(cpu_state_t *cpu, string op1, string op2);
void exec_int(cpu_state_t *cpu, string op1);
void exec_xor(cpu_state_t *cpu, string op1, string op2);
#endif // !CPU_HPP
