#include "cpu.hpp"
#include "utils.hpp"

types detect_type(const string &name) {
  if (name[0] == '[' && name.back() == ']') {
    return MEMORY;
  } else if (name.find_first_not_of("0123456789ABCDEFabcdef") == string::npos) {
    return IMMEDIATE;
  } else {
    return REGISTER;
  }
}

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

void exec_parsed(cpu_state *cpu, instruction_info info) {
  if (info.mnemonic == "MOV") {
    exec_mov(cpu, info.op1, info.op2);
  }
  if (info.mnemonic == "INT") {
    exec_int(cpu, info.op1);
  }
};

void exec_mov(cpu_state *cpu, string dst, string src) {
  types dst_type = detect_type(dst);
  types src_type = detect_type(src);

  if (dst_type == REGISTER && src_type == IMMEDIATE) {
    uint8_t reg_index = parse_reg_name(dst);
    uint16_t imm_val = parse_hex_string(src);

    set_reg16(cpu, reg_index, imm_val);
  }
};

void exec_int(cpu_state *cpu, string op1);
