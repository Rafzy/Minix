#ifndef CPU_HPP
#define CPU_HPP

#include "opcode.hpp"
#include <cstdint>

#define MEMORY_SIZE (1 << 20);

typedef enum {
  REGISTER = 0,
  IMMEDIATE,
  MEMORY,
  REGISTER_LOW,
  REGISTER_HIGH
} types;

typedef struct {
  // std::unordered_map<uint32_t, uint8_t> data;
  uint8_t data[(1 << 20)];
} memory_t;

typedef struct {
  uint16_t registers[17];
  memory_t *memory;
  bool running;
} cpu_state_t;

types detect_type(const string &name);

void init_cpu(cpu_state_t *cpu);
void init_stack(cpu_state_t *cpu, int argc, char *argv[]);
uint16_t get_reg16(cpu_state_t *cpu, uint8_t reg);
uint8_t get_reg8_h(cpu_state_t *cpu, uint8_t reg);
uint8_t get_reg8_l(cpu_state_t *cpu, uint8_t reg);
void set_reg16(cpu_state_t *cpu, uint8_t reg, uint16_t val);
void set_reg8_h(cpu_state_t *cpu, uint8_t reg, uint8_t val);
void set_reg8_l(cpu_state_t *cpu, uint8_t reg, uint8_t val);
// Executions
void exec_parsed(cpu_state_t *cpu, instruction_info &info);
void exec_mov(cpu_state_t *cpu, string op1, string op2);
void exec_int(cpu_state_t *cpu, string op1);
void exec_xor(cpu_state_t *cpu, string op1, string op2);
void exec_or(cpu_state_t *cpu, string op1, string op2);
void exec_lea(cpu_state_t *cpu, string op1, string op2);
void exec_add(cpu_state_t *cpu, string op1, string op2);
void exec_sub(cpu_state_t *cpu, string op1, string op2);
void exec_cmp(cpu_state_t *cpu, string op1, string op2);
void exec_jnb(cpu_state_t *cpu, string op1);
void exec_test(cpu_state_t *cpu, string op1, string op2);
void exec_jne(cpu_state_t *cpu, string op1);
void exec_je(cpu_state_t *cpu, string op1);
void exec_jnl(cpu_state_t *cpu, string op1);
void exec_jl(cpu_state_t *cpu, string op1);
void exec_push(cpu_state_t *cpu, string op1);
void exec_call(cpu_state_t *cpu, string op1);
void exec_jmp(cpu_state_t *cpu, string op1);
void exec_jmp_short(cpu_state_t *cpu, string op1);
void exec_pop(cpu_state_t *cpu, string op1);
void exec_ret(cpu_state_t *cpu);
void exec_dec(cpu_state_t *cpu, string op1);
void exec_cbw(cpu_state_t *cpu);
void exec_inc(cpu_state_t *cpu, string op1);
void exec_and(cpu_state_t *cpu, string op1, string op2);
#endif // !CPU_HPP
