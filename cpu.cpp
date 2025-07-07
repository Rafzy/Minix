#include "cpu.hpp"
#include "interrupts.hpp"
#include "registers.hpp"
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

void init_cpu(cpu_state_t *cpu) {
  for (int i = 0; i < 14; i++) {
    cpu->registers[i] = 0;
  }
  cpu->registers[CS] = 0x1000;
  cpu->registers[DS] = 0x2000;
  cpu->registers[SS] = 0x3000;
  cpu->registers[ES] = 0x2000;

  cpu->registers[SP] = 0xFFFF;
  cpu->registers[IP] = 0x0000;

  // allocate for memory
  cpu->memory = (memory_t *)malloc(sizeof(memory_t));
}

uint16_t get_reg16(cpu_state_t *cpu, uint8_t reg) {
  if (reg > 7)
    return 0;
  return cpu->registers[reg];
}

void set_reg16(cpu_state_t *cpu, uint8_t reg, uint16_t val) {
  if (reg > 7)
    return;
  cpu->registers[reg] = val;
}

void exec_parsed(cpu_state_t *cpu, instruction_info &info) {
  if (info.mnemonic == "mov") {
    exec_mov(cpu, info.op1, info.op2);
  }
  if (info.mnemonic == "int") {
    exec_int(cpu, info.op1);
  }
  if (info.mnemonic == "xor") {
    exec_xor(cpu, info.op1, info.op2);
  }
};

void exec_mov(cpu_state_t *cpu, string dst, string src) {
  types dst_type = detect_type(dst);
  types src_type = detect_type(src);

  if (dst_type == REGISTER && src_type == IMMEDIATE) {
    uint8_t reg_index = parse_reg_name(dst);
    uint16_t imm_val = parse_hex_string(src);

    set_reg16(cpu, reg_index, imm_val);
  }

  if (dst_type == REGISTER && src_type == REGISTER) {
    uint8_t reg_src = parse_reg_name(src);
    uint8_t reg_dst = parse_reg_name(dst);
    uint16_t reg_src_val = get_reg16(cpu, reg_src);

    set_reg16(cpu, reg_dst, reg_src_val);
  }

  if (dst_type == REGISTER && src_type == MEMORY) {
    uint8_t reg_dst = parse_reg_name(dst);
    uint16_t mem_addr = parse_memory(cpu, src);
    uint32_t full_addr = (cpu->registers[DS] << 4) + mem_addr;
    uint16_t mem_val = le_16(cpu->memory->data, full_addr);

    set_reg16(cpu, reg_dst, mem_val);
  }
};

void exec_int(cpu_state_t *cpu, string op1) {
  if (op1 == "20") {
    exec_int20(cpu);
  }
};

void exec_xor(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);

    cpu->registers[op1_index] =
        (cpu->registers[op1_index] & cpu->registers[op2_index]);
    // Set Z flag
    if (cpu->registers[op1_index] == 0) {
      cpu->registers[ZF] = 1;
    }
  }
}
