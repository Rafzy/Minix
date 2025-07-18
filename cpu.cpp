#include "cpu.hpp"
#include "interrupts.hpp"
#include "parser.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>

// types detect_type(const string &name) {
//   if (name[0] == '[' && name.back() == ']') {
//     return MEMORY;
//   } else if (name.find_first_not_of("0123456789ABCDEFabcdef") ==
//   string::npos) {
//     return IMMEDIATE;
//   } else {
//     return REGISTER;
//   }
// }

types detect_type(const string &name) {
  if (name[0] == '[' && name.back() == ']') {
    // Check if the memory reference contains 'L' or 'H'
    // if (name.find('L') != string::npos) {
    //   return MEMORY_LOW;
    // } else if (name.find('H') != string::npos) {
    //   return MEMORY_HIGH;
    // } else {
    //   return MEMORY;
    // }
    return MEMORY;
  } else if (name.find_first_not_of("0123456789ABCDEFabcdef") == string::npos) {
    return IMMEDIATE;
  } else {
    if (name.find('L') != string::npos) {
      return REGISTER_LOW;
    } else if (name.find('H') != string::npos) {
      return REGISTER_HIGH;
    } else {

      return REGISTER;
    }
    // return REGISTER;
  }
}

void init_stack(cpu_state_t *cpu, int argc, char **argv) {

  string env = "PATH=/bin:/usr/bin";

  uint16_t ss_addr = cpu->registers[SS];

  uint16_t addresses[argc];
  int addr_index = 0;

  // set env's length and adjust SP register
  int env_len = env.length() + 1;
  cpu->registers[SP] -= env_len;
  uint16_t env_addr = cpu->registers[SP];

  for (int i = 0; i < env_len; ++i) {
    set_mem(cpu, ss_addr, cpu->registers[SP] + i, env[i]);
  }

  for (int i = 0; i < argc; ++i) {
    int len = strlen(argv[i]) + 1;
    cpu->registers[SP] -= len;

    addresses[addr_index++] = cpu->registers[SP];

    for (int j = 0; j < len; ++j) {
      // uint32_t address = (ss_addr << 4) + cpu->registers[SP] + j;
      // cpu->memory->data[address] = argv[i][j];
      set_mem(cpu, ss_addr, cpu->registers[SP] + j, argv[i][j]);
    }
  }

  cpu->registers[SP] -= 2;
  // delimiter
  set_mem(cpu, ss_addr, cpu->registers[SP], 0);
  set_mem(cpu, ss_addr, cpu->registers[SP] + 1, 0);

  // Add env address
  cpu->registers[SP] -= 2;
  set_mem(cpu, ss_addr, cpu->registers[SP], env_addr & 0xFF);
  set_mem(cpu, ss_addr, cpu->registers[SP] + 1, env_addr >> 8);

  // I'm not sure whether the 0 should be 16 bits or 8 bits
  cpu->registers[SP] -= 2;
  // delimiter
  set_mem(cpu, ss_addr, cpu->registers[SP], 0);
  set_mem(cpu, ss_addr, cpu->registers[SP] + 1, 0);

  for (int i = 0; i < addr_index; ++i) {
    cpu->registers[SP] -= 2;
    uint16_t argv_address = addresses[i];

    // uint32_t address = (ss_addr << 4) + cpu->registers[SP];
    // cpu->memory->data[address] = argv_address & 0xFF;
    // cpu->memory->data[address + 1] = argv_address >> 8;
    set_mem(cpu, ss_addr, cpu->registers[SP], argv_address & 0xFF);
    set_mem(cpu, ss_addr, cpu->registers[SP] + 1, argv_address >> 8);
  }

  cpu->registers[SP] -= 2;
  // uint32_t phys_address = (ss_addr << 4) + cpu->registers[SP];
  // cpu->memory->data[address] = argc & 0xFF;
  // cpu->memory->data[address + 1] = argc >> 8;
  set_mem(cpu, ss_addr, cpu->registers[SP], argc & 0xFF);
  set_mem(cpu, ss_addr, cpu->registers[SP] + 1, argc >> 8);
}

void init_cpu(cpu_state_t *cpu) {
  for (int i = 0; i < 14; i++) {
    cpu->registers[i] = 0;
  }
  cpu->registers[CS] = 0x1000;
  cpu->registers[DS] = 0x2000;
  cpu->registers[SS] = 0x4000;
  cpu->registers[ES] = 0x2000;

  cpu->registers[SP] = 0xFFFF;
  cpu->registers[IP] = 0x0000;

  cpu->registers[CF] = 0;
  cpu->registers[SF] = 0;
  cpu->registers[ZF] = 0;
  cpu->registers[OF] = 0;

  // allocate for memory
  cpu->memory = (memory_t *)malloc(sizeof(memory_t));

  cpu->running = true;
}

uint16_t get_reg16(cpu_state_t *cpu, uint8_t reg) {
  if (reg > 7)
    return 0;
  return cpu->registers[reg];
}

uint8_t get_reg8_l(cpu_state_t *cpu, uint8_t reg) {
  if (reg > 7)
    return 0;
  return cpu->registers[reg] & 0xFF;
}

uint8_t get_reg8_h(cpu_state_t *cpu, uint8_t reg) {
  if (reg > 7)
    return 0;

  return cpu->registers[reg] >> 8;
}

void set_reg16(cpu_state_t *cpu, uint8_t reg, uint16_t val) {
  if (reg > 7)
    return;
  cpu->registers[reg] = val;
}

void set_reg8_l(cpu_state_t *cpu, uint8_t reg, uint8_t val) {
  if (reg > 7)
    return;

  uint16_t reg_val = cpu->registers[reg];
  reg_val = (reg_val & 0xFF00) | val;

  cpu->registers[reg] = reg_val;
}

void set_reg8_h(cpu_state_t *cpu, uint8_t reg, uint8_t val) {
  if (reg > 7)
    return;

  uint16_t reg_val = cpu->registers[reg];
  reg_val = (reg_val & 0x00FF) | (val << 4);

  cpu->registers[reg] = reg_val;
}

void exec_parsed(cpu_state_t *cpu, instruction_info &info) {
  if (info.mnemonic == "mov") {
    exec_mov(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "int") {
    exec_int(cpu, info.op1);
  } else if (info.mnemonic == "xor") {
    exec_xor(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "lea") {
    exec_lea(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "add") {
    exec_add(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "cmp") {
    exec_cmp(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "jnb") {
    exec_jnb(cpu, info.op1);
  } else if (info.mnemonic == "test") {
    exec_test(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "jne") {
    exec_jne(cpu, info.op1);
  } else if (info.mnemonic == "je") {
    exec_je(cpu, info.op1);
  } else if (info.mnemonic == "jnl") {
    exec_jnl(cpu, info.op1);
  } else if (info.mnemonic == "jl") {
    exec_jl(cpu, info.op1);
  } else if (info.mnemonic == "push") {
    exec_push(cpu, info.op1);
  } else if (info.mnemonic == "call") {
    exec_call(cpu, info.op1);
  } else if (info.mnemonic == "jmp") {
    exec_jmp(cpu, info.op1);
  } else if (info.mnemonic == "sub") {
    exec_sub(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "jmp short") {
    exec_jmp_short(cpu, info.op1);
  } else if (info.mnemonic == "pop") {
    exec_pop(cpu, info.op1);
  } else if (info.mnemonic == "ret") {
    exec_ret(cpu, info.op1);
  } else if (info.mnemonic == "or") {
    exec_or(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "dec") {
    exec_dec(cpu, info.op1);
  } else if (info.mnemonic == "cbw") {
    exec_cbw(cpu);
  } else if (info.mnemonic == "inc") {
    exec_inc(cpu, info.op1);
  } else if (info.mnemonic == "and") {
    exec_and(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "neg") {
    exec_neg(cpu, info.op1);
  } else if (info.mnemonic == "jb") {
    exec_jb(cpu, info.op1);
  } else if (info.mnemonic == "jle") {
    exec_jle(cpu, info.op1);
  } else if (info.mnemonic == "jnbe") {
    exec_jnbe(cpu, info.op1);
  } else if (info.mnemonic == "jbe") {
    exec_jbe(cpu, info.op1);
  } else if (info.mnemonic == "shl") {
    exec_shl(cpu, info.op1, info.op2);
  } else if (info.mnemonic == "jnle") {
    exec_jnle(cpu, info.op1);
  } else if (info.mnemonic == "cwd") {
    exec_cwd(cpu);
  } else if (info.mnemonic == "div") {
    exec_div(cpu, info.op1);
  } else if (info.mnemonic == "xchg") {
    exec_xchg(cpu, info.op1);
  } else if (info.mnemonic == "sar") {
    exec_sar(cpu, info.op1, info.op2);
  }

  else {
    cpu->running = false;
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

  else if (dst_type == REGISTER && src_type == REGISTER) {
    uint8_t reg_src = parse_reg_name(src);
    uint8_t reg_dst = parse_reg_name(dst);
    uint16_t reg_src_val = get_reg16(cpu, reg_src);

    set_reg16(cpu, reg_dst, reg_src_val);
  }

  else if (dst_type == REGISTER && src_type == MEMORY) {
    uint8_t reg_dst = parse_reg_name(dst);
    uint16_t mem_addr = parse_memory(cpu, src);

    uint8_t segment;
    (mem_addr > 0xf000) ? segment = SS : segment = DS;

    uint16_t mem_val = get_mem_16(cpu, cpu->registers[segment], mem_addr);

    // printf("FROM MOV ADDR: %04x\n", mem_addr);
    // printf("FROM MOV VAL: %04x\n", mem_val);

    set_reg16(cpu, reg_dst, mem_val);
  }

  else if (dst_type == MEMORY && src_type == REGISTER) {
    uint8_t reg_src = parse_reg_name(src);
    uint16_t mem_addr = parse_memory(cpu, dst);
    uint16_t src_val = get_reg16(cpu, reg_src);

    uint8_t segment;
    (mem_addr > 0xf000) ? segment = SS : segment = DS;

    // set_mem(cpu, cpu->registers[segment], mem_addr, src_val & 0xff);
    // set_mem(cpu, cpu->registers[segment], mem_addr + 1, src_val >> 8);
    set_mem16(cpu, cpu->registers[segment], mem_addr, src_val);
    // printf("MOV ADDR: %04x\n", mem_addr);
    // printf("RESULT AFTER MOV MEMORY: %04x\n",
    //        get_mem_16(cpu, cpu->registers[segment], mem_addr));
  } else if (dst_type == MEMORY && src_type == IMMEDIATE) {
    uint16_t dst_addr = parse_memory(cpu, dst);
    uint16_t src_val = parse_hex_string(src);

    uint8_t segment;
    (dst_addr > 0xf000) ? segment = SS : segment = DS;

    set_mem16(cpu, cpu->registers[segment], dst_addr, src_val);
  }

  else if (dst_type == REGISTER_LOW && src_type == IMMEDIATE) {
    uint8_t dst_index = parse_reg_name(dst);
    uint8_t src_val = parse_hex_string(src);

    set_reg8_l(cpu, dst_index, src_val);
  }

  else if (dst_type == REGISTER_HIGH && src_type == IMMEDIATE) {
    uint8_t dst_index = parse_reg_name(dst);
    uint8_t src_val = parse_hex_string(src);

    set_reg8_h(cpu, dst_index, src_val);
  }

  else if (dst_type == REGISTER_LOW && src_type == MEMORY) {
    uint8_t dst_index = parse_reg_name(dst);
    uint16_t src_addr = parse_memory(cpu, src);

    uint8_t segment;
    (src_addr > 0xf000) ? segment = SS : segment = DS;

    uint8_t src_val = get_mem_8(cpu, cpu->registers[segment], src_addr);

    set_reg8_l(cpu, dst_index, src_val);
  }

  else if (dst_type == REGISTER_HIGH && src_type == MEMORY) {
    uint8_t dst_index = parse_reg_name(dst);
    uint16_t src_addr = parse_memory(cpu, src);

    uint8_t segment;
    (src_addr > 0xf000) ? segment = SS : segment = DS;

    uint8_t src_val = get_mem_8(cpu, segment, src_addr);

    set_reg8_h(cpu, dst_index, src_val);
  } else if (dst_type == MEMORY && src_type == REGISTER_LOW) {
    uint8_t src_index = parse_reg_name(src);
    uint16_t src_val = get_reg8_l(cpu, src_index);
    uint16_t dst_addr = parse_memory(cpu, dst);

    uint8_t segment;
    (dst_addr > 0xf000) ? segment = SS : segment = DS;

    set_mem(cpu, cpu->registers[segment], dst_addr, src_val);
  }

  else {
    cpu->running = false;
  }
};

void exec_int(cpu_state_t *cpu, string op1) {
  // Syscalls
  if (op1 == "20") {
    exec_int20(cpu);
  }
};

void exec_xor(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  cpu->registers[OF] = 0;
  cpu->registers[CF] = 0;

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);

    cpu->registers[op1_index] =
        (cpu->registers[op1_index] ^ cpu->registers[op2_index]);
    // Set Z flag
    (cpu->registers[op1_index] == 0) ? cpu->registers[ZF] = 1
                                     : cpu->registers[ZF] = 0;
    // Set S Flag
    ((cpu->registers[op1_index] >> 15) == 1) ? cpu->registers[SF] = 1
                                             : cpu->registers[SF] = 0;
  } else if (op1_type == REGISTER_LOW && op2_type == REGISTER_LOW) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);
    uint8_t op1_val = get_reg8_l(cpu, op1_index);
    uint8_t op2_val = get_reg8_l(cpu, op2_index);

    uint8_t result = op1_val ^ op2_val;

    // printf("XOR LOW REG RESULT: %04x\n", result);

    (result == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
    (result >> 7) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
    set_reg8_l(cpu, op1_index, result);
  } else if (op1_type == REGISTER_HIGH && op2_type == REGISTER_HIGH) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);
    uint8_t op1_val = get_reg8_h(cpu, op1_index);
    uint8_t op2_val = get_reg8_h(cpu, op2_index);

    uint8_t result = op1_val ^ op2_val;

    // printf("XOR HIGH REG RESULT: %04x\n", result);

    (result == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
    (result >> 7) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
    set_reg8_h(cpu, op1_index, result);
  } else {
    cpu->running = false;
  }
}

void exec_or(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t result;

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);

    result = cpu->registers[op1_index] | cpu->registers[op2_index];
    set_reg16(cpu, op1_index, result);
  } else if (op1_type == MEMORY && op2_type == IMMEDIATE) {
    uint16_t op1_addr = parse_memory(cpu, op1);
    uint8_t segment;
    (op1_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_addr);
    uint16_t op2_val = parse_hex_string(op2);

    result = op1_val | op2_val;
    set_mem16(cpu, cpu->registers[segment], op1_addr, result);
  } else if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = parse_hex_string(op2);

    result = op1_val | op2_val;
    set_reg16(cpu, op1_index, result);
  } else if (op1_type == REGISTER && op2_type == MEMORY) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_addr = parse_memory(cpu, op2);
    uint8_t segment;
    (op2_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op2_val = get_mem_16(cpu, cpu->registers[segment], op2_addr);

    result = op1_val | op2_val;
    set_reg16(cpu, op1_index, result);
  } else {
    cpu->running = false;
  }
  // Update registers
  cpu->registers[OF] = 0;
  cpu->registers[CF] = 0;
  result == 0 ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_lea(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == MEMORY) {
    uint16_t reg_index = parse_reg_name(op1);
    uint16_t mem_addr = parse_memory(cpu, op2);

    set_reg16(cpu, reg_index, mem_addr);
  } else {
    cpu->running = false;
  }
}

void exec_add(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t total;

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint16_t op1_index = parse_reg_name(op1);
    uint16_t op2_index = parse_reg_name(op2);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = get_reg16(cpu, op2_index);
    total = op1_val + op2_val;

    if (total < op1_val || total < op2_val) {
      cpu->registers[CF] = 1;
    } else {
      cpu->registers[CF] = 0;
    }
    set_reg16(cpu, op1_index, total);
  } else if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = cpu->registers[op1_index];
    uint16_t op2_val = parse_hex_string(op2);
    total = op1_val + op2_val;

    if (total < op1_val || total < op2_val) {
      cpu->registers[CF] = 1;
    } else {
      cpu->registers[CF] = 0;
    }
    cpu->registers[op1_index] = total;
  } else if (op1_type == REGISTER && op2_type == MEMORY) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_addr = parse_memory(cpu, op2);

    uint8_t segment;
    (op2_addr > 0xf000) ? segment = SS : segment = DS;

    uint16_t op2_val = get_mem_16(cpu, cpu->registers[segment], op2_addr);

    total = op1_val + op2_val;

    if (total < op1_val || total < op2_val) {
      cpu->registers[CF] = 1;
    } else {
      cpu->registers[CF] = 0;
    }

    set_reg16(cpu, op1_index, total);
  } else if (op1_type == MEMORY && op2_type == IMMEDIATE) {
    uint16_t op1_addr = parse_memory(cpu, op1);
    uint8_t segment;
    (op1_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_addr);
    uint16_t op2_val = parse_hex_string(op2);

    total = op1_val + op2_val;
    if (total < op1_val || total < op2_val) {
      cpu->registers[CF] = 1;
    } else {
      cpu->registers[CF] = 0;
    }
    set_mem16(cpu, cpu->registers[segment], op1_addr, total);
  } else if (op1_type == MEMORY && op2_type == REGISTER) {
    uint16_t op1_addr = parse_memory(cpu, op1);
    uint8_t segment;
    (op1_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_addr);

    uint8_t op2_index = parse_reg_name(op2);
    uint16_t op2_val = get_reg16(cpu, op2_index);

    total = op1_val + op2_val;
    if (total < op1_val || total < op2_val) {
      cpu->registers[CF] = 1;
    } else {
      cpu->registers[CF] = 0;
    }
    set_mem16(cpu, cpu->registers[segment], op1_addr, total);
  } else {
    cpu->running = false;
  }
  // Update Z Flag
  (total == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  // Update S Flag
  (total & 0x8000) != 0 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_sub(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t result;

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = parse_hex_string(op2);

    result = op1_val - op2_val;

    (op1_val < op2_val) ? cpu->registers[CF] = 1 : cpu->registers[CF] = 0;

    cpu->registers[op1_index] = result;
  } else if (op1_type == REGISTER && op2_type == MEMORY) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_addr = parse_memory(cpu, op2);
    uint8_t segment;
    (op2_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op2_val = get_mem_16(cpu, cpu->registers[segment], op2_addr);

    result = op1_val - op2_val;

    (op1_val < op2_val) ? cpu->registers[CF] = 1 : cpu->registers[CF] = 0;

    set_reg16(cpu, op1_index, result);
  } else {
    cpu->running = false;
  }
  // Update Flags
  (result == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  ((result & 0x8000) != 0) ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_cmp(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t op1_val;
  uint16_t op2_val;

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint16_t eax_index = parse_reg_name(op1);
    uint16_t ebx_index = parse_reg_name(op2);

    op1_val = get_reg16(cpu, eax_index);
    op2_val = get_reg16(cpu, ebx_index);

    // if (get_reg16(cpu, eax_index) < get_reg16(cpu, ebx_index)) {
    //   cpu->registers[CF] = 1;
    // } else if (get_reg16(cpu, eax_index) >= get_reg16(cpu, ebx_index)) {
    //   cpu->registers[CF] = 0;
    // }
    //
    // if (cpu->registers[eax_index] - cpu->registers[ebx_index] == 0) {
    //   cpu->registers[ZF] = 1;
    // } else {
    //   cpu->registers[ZF] = 0;
    // }
  }

  else if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint16_t op1_index = parse_reg_name(op1);
    op1_val = get_reg16(cpu, op1_index);
    op2_val = parse_hex_string(op2);
    int16_t op2_val_se = (int16_t)op2_val;
    // int16_t op2_val_se = (int8_t)op2_val;
    // printf("COMPARE SIGN EXTENDED : %d\n", op2_val_se);

    (op1_val < op2_val_se) ? cpu->registers[CF] = 1 : cpu->registers[CF] = 0;
    (op1_val - op2_val_se == 0) ? cpu->registers[ZF] = 1
                                : cpu->registers[ZF] = 0;
    int16_t result = op1_val - op2_val_se;
    (result & 0x8000) != 0 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
    return;
  }

  else if (op1_type == MEMORY && op2_type == IMMEDIATE) {
    uint16_t mem_op1 = parse_memory(cpu, op1);

    uint8_t segment;
    (mem_op1 > 0xf000) ? segment = SS : segment = DS;

    op2_val = parse_hex_string(op2);
    op1_val = get_mem_16(cpu, cpu->registers[segment], mem_op1);

    // printf("CMP MEM VAL ADDRESS: %04x\n", mem_op1);
    // printf("CMP MEM VAL: %04x\n", op1_val);
  }

  else if (op1_type == MEMORY && op2_type == REGISTER) {
    uint16_t mem_op1 = parse_memory(cpu, op1);
    uint8_t segment;
    (mem_op1 > 0xf000) ? segment = SS : segment = DS;
    op1_val = get_mem_16(cpu, cpu->registers[segment], mem_op1);

    // printf("CMP MEM VAL: %04x\n", op1_val);

    uint8_t op2_index = parse_reg_name(op2);
    op2_val = get_reg16(cpu, op2_index);
    // printf("CMP REG VAL: %05x\n", op2_val);
  }

  else if (op1_type == REGISTER && op2_type == MEMORY) {
    uint8_t op1_index = parse_reg_name(op1);
    op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_addr = parse_memory(cpu, op2);
    uint8_t segment;
    (op2_addr > 0xf000) ? segment = SS : segment = DS;
    op2_val = get_mem_16(cpu, cpu->registers[segment], op2_addr);
    // printf("CMP REG MEM: %04x %04x\n", op1_val, op2_val);
  }

  else {
    cpu->running = false;
  }

  // Update flags
  if (op1_val < op2_val) {
    cpu->registers[CF] = 1;
  } else if (op1_val >= op2_val) {
    cpu->registers[CF] = 0;
  }
  if (op1_val - op2_val == 0) {
    cpu->registers[ZF] = 1;
  } else {
    cpu->registers[ZF] = 0;
  }
  uint16_t result = op1_val - op2_val;
  (result & 0x8000) != 0 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_jnb(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op_val = parse_hex_string(op1);

    if (cpu->registers[CF] == 0) {
      cpu->registers[IP] = op_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_test(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t result;

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint16_t op1_index = parse_reg_name(op1);
    // uint16_t op2_index = parse_reg_name(op2);
    uint16_t op2_val = parse_hex_string(op2);
    result = get_reg16(cpu, op1_index) & op2_val;
  } else if (op1_type == REGISTER && op2_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = get_reg16(cpu, op2_index);

    result = op1_val & op2_val;

  }

  else if (op1_type == REGISTER_LOW && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op1_val = get_reg8_l(cpu, op1_index);
    uint8_t op2_val = parse_hex_string(op2);

    result = op1_val & op2_val;
  } else if (op1_type == REGISTER_HIGH && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op1_val = get_reg8_h(cpu, op1_index);
    uint8_t op2_val = parse_hex_string(op2);

    result = op1_val & op2_val;
  } else if (op1_type == MEMORY && op2_type == IMMEDIATE) {
    uint16_t op1_addr = parse_memory(cpu, op1);
    uint16_t op2_val = parse_hex_string(op2);

    uint8_t segment;
    (op1_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_addr);

    // printf("TEST MEM ADDR: %04x\n", op1_addr);
    // printf("TEST MEM VAL: %04x\n", op1_val);
    result = op1_val & op2_val;
  } else {
    cpu->running = false;
  }
  // Update C Flage
  cpu->registers[CF] = 0;
  // Update Z Flag
  result == 0 ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  // Update S Flag
  (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_jne(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[ZF] == 0) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_je(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[ZF] == 1) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_jnl(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[OF] == cpu->registers[SF]) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_jl(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[OF] != cpu->registers[SF]) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_jb(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[CF] == 1) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_jle(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[ZF] == 1 || cpu->registers[SF] != cpu->registers[OF]) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_jnbe(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[CF] == 0 && cpu->registers[ZF] == 0) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_jbe(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {

    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[CF] == 1 || cpu->registers[ZF] == 1) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}
void exec_jnle(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {

    uint16_t op1_val = parse_hex_string(op1);

    if (cpu->registers[ZF] == 0 && cpu->registers[SF] == cpu->registers[OF]) {
      cpu->registers[IP] = op1_val;
      cpu->registers[IP] -= 2;
    }
  } else {
    cpu->running = false;
  }
}

void exec_push(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    // if (cpu->registers[op1_index] >= 2) {
    //   cpu->registers[op1_index] -= 2;
    // }

    push_stack(cpu, cpu->registers[op1_index]);
  }

  else if (op1_type == MEMORY) {
    uint16_t mem_addr = parse_memory(cpu, op1);
    uint8_t segment;
    (mem_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], mem_addr);
    push_stack(cpu, op1_val);
    // printf("VAL AFTER PUSH: %04x\n",
    //        get_mem_16(cpu, cpu->registers[SS], cpu->registers[SP]));
  } else {
    cpu->running = false;
  }
}

void exec_call(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  uint16_t op1_val;
  if (op1_type == IMMEDIATE) {
    op1_val = parse_hex_string(op1);
    cpu->registers[IP] += 3;
    push_stack(cpu, cpu->registers[IP]);
    cpu->registers[IP] = op1_val;

    // This is stupid
    cpu->registers[IP] -= 3;
  } else if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    op1_val = get_reg16(cpu, op1_index);
    cpu->registers[IP] += 2;
    push_stack(cpu, cpu->registers[IP]);
    cpu->registers[IP] = op1_val;

    // This is stupid
    cpu->registers[IP] -= 2;
  } else {
    cpu->running = false;
  }
}

void exec_jmp(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  uint16_t op1_val;

  if (op1_type == IMMEDIATE) {
    op1_val = parse_hex_string(op1);
    cpu->registers[IP] = op1_val;
    cpu->registers[IP] -= 3;
  } else if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    op1_val = get_reg16(cpu, op1_index);
    cpu->registers[IP] = op1_val;
    cpu->registers[IP] -= 2;
  } else {
    cpu->running = false;
  }
}

void exec_jmp_short(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == IMMEDIATE) {
    uint16_t op1_val = parse_hex_string(op1);

    cpu->registers[IP] = op1_val;
    cpu->registers[IP] -= 2;
  } else {
    cpu->running = false;
  }
}

void exec_pop(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    cpu->registers[op1_index] = pop_stack(cpu);
  } else {
    cpu->running = false;
  }
}

void exec_ret(cpu_state_t *cpu, string op1) {
  if (op1 == "") {
    cpu->registers[IP] = pop_stack(cpu);
    cpu->registers[IP] -= 1;
  } else {
    uint16_t op1_val = parse_hex_string(op1);
    cpu->registers[IP] = pop_stack(cpu);
    // printf("RETURN VAL: %04x\n", cpu->registers[IP]);
    cpu->registers[SP] += op1_val;
    cpu->registers[IP] -= 3;
  }
}

void exec_dec(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  uint16_t result;

  if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    cpu->registers[op1_index] -= 1;
    cpu->registers[op1_index] == 0 ? cpu->registers[ZF] = 1
                                   : cpu->registers[ZF] = 0;
    (cpu->registers[op1_index] >> 15 == 1) ? cpu->registers[SF] = 1
                                           : cpu->registers[SF] = 0;
    return;
  } else if (op1_type == MEMORY) {
    uint16_t op1_addr = parse_memory(cpu, op1);
    uint8_t segment;
    (op1_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_addr);
    result = op1_val - 1;
    set_mem16(cpu, cpu->registers[segment], op1_addr, result);
  } else {
    cpu->running = false;
  }
  result == 0 ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  (result & 0x8000) != 0 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_cbw(cpu_state_t *cpu) {
  uint8_t al = get_reg8_l(cpu, AX);
  uint16_t al_extended = (uint16_t)(int16_t)(int8_t)al;
  uint16_t sign = al_extended >> 15;
  set_reg16(cpu, AX, al_extended);
}

void exec_inc(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  uint16_t result;

  if (op1_type == REGISTER) {
    uint16_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    result = op1_val + 1;

    set_reg16(cpu, op1_index, result);
  } else if (op1_type == MEMORY) {
    uint16_t op1_mem = parse_memory(cpu, op1);

    uint8_t segment;
    op1_mem > 0xf000 ? segment = SS : segment = DS;

    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_mem);
    result = op1_val + 1;

    set_mem16(cpu, cpu->registers[segment], op1_mem, result);
    // printf("MEM VAL AFTER INC: %04x\n",
    //        get_mem_16(cpu, cpu->registers[segment], op1_mem));
  } else {
    cpu->running = false;
  }

  result == 0 ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_and(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t result;

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint16_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = parse_hex_string(op2);

    result = op1_val & op2_val;
    set_reg16(cpu, op1_index, result);
  } else if (op1_type == MEMORY && op2_type == IMMEDIATE) {
    uint16_t op1_addr = parse_memory(cpu, op1);
    uint16_t op2_val = parse_hex_string(op2);

    uint8_t segment;
    (op1_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[segment], op1_addr);

    result = op1_val & op2_val;
    set_mem16(cpu, segment, op1_addr, result);
  } else if (op1_type == REGISTER && op2_type == MEMORY) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_addr = parse_memory(cpu, op2);
    uint8_t segment;
    (op2_addr > 0xf000) ? segment = SS : segment = DS;
    uint16_t op2_val = get_mem_16(cpu, cpu->registers[segment], op2_addr);

    result = op1_val & op2_val;
    set_reg16(cpu, op1_index, result);
  } else {
    cpu->running = false;
  }

  cpu->registers[OF] = 0;
  cpu->registers[CF] = 0;
  (result == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_neg(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);
  uint16_t result;

  if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);

    (op1_val == 0) ? cpu->registers[CF] = 0 : cpu->registers[CF] = 1;
    result = 0 - op1_val;
    set_reg16(cpu, op1_index, result);
  } else {
    cpu->running = false;
  }
  (result == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
  (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
}

void exec_shl(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t result;

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = parse_hex_string(op2);

    uint16_t count = op2_val &= 0x1F;

    if (count >= 16) {
      cpu->registers[CF] = (count == 16) ? (op1_val & 0x8000) != 0 : 0;
      cpu->registers[ZF] = 1;
      cpu->registers[SF] = 0;
      set_reg16(cpu, op1_index, 0);
      return;
    }

    cpu->registers[CF] = (op1_val & (1 << (16 - count))) != 0;

    result = op1_val << count;

    cpu->registers[ZF] = (result == 0);
    cpu->registers[SF] = (result & 0x80000) != 0;

    set_reg16(cpu, op1_index, result);
    return;
  } else if (op1_type == REGISTER && REGISTER_LOW) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = get_reg8_l(cpu, op2_index);

    uint16_t count = op2_val &= 0x1F;

    if (count >= 16) {
      cpu->registers[CF] = (count == 16) ? (op1_val & 0x8000) != 0 : 0;
      cpu->registers[ZF] = 1;
      cpu->registers[SF] = 0;
      set_reg16(cpu, op1_index, 0);
      return;
    }

    cpu->registers[CF] = (op1_val & (1 << (16 - count))) != 0;

    result = op1_val << count;

    cpu->registers[ZF] = (result == 0);
    cpu->registers[SF] = (result & 0x80000) != 0;

    set_reg16(cpu, op1_index, result);
    return;

  } else {
    cpu->running = false;
  }
}

void exec_sar(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);
  uint16_t result;

  if (op1_type == REGISTER && op2_type == REGISTER_LOW) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = get_reg8_l(cpu, op2_index);

    uint16_t count = op2_val & 0x1F;

    if (count >= 16) {
      if (op1_val & 0x8000) {
        result = 0xFFFF;
        cpu->registers[CF] = 1; // Last bit shifted out is 1
        cpu->registers[ZF] = 0; // Result is not zero
        cpu->registers[SF] = 1; // Sign bit is 1
      } else {
        // Positive number - fill with 0s
        result = 0x0000;
        cpu->registers[CF] = 0; // Last bit shifted out is 0
        cpu->registers[ZF] = 1; // Result is zero
        cpu->registers[SF] = 0; // Sign bit is 0
      }
      set_reg16(cpu, op1_index, result);
      return;
    }

    if (count > 0) {
      cpu->registers[CF] = (op1_val & (1 << (count - 1))) != 0;
    }

    int16_t signed_val = (int16_t)op1_val;
    result = (uint16_t)(signed_val >> count);

    cpu->registers[ZF] = (result == 0);
    cpu->registers[SF] = (result & 0x8000) != 0;

    set_reg16(cpu, op1_index, result);
  }
}

void exec_cwd(cpu_state_t *cpu) {
  uint16_t ax = get_reg16(cpu, AX);
  uint16_t ax_extended = (uint16_t)(int16_t)ax;
  uint16_t sign = ax_extended >> 15;
  set_reg16(cpu, DX, sign);
}

void exec_div(cpu_state_t *cpu, string op1) {
  uint16_t ax = get_reg16(cpu, AX);
  uint16_t dx = get_reg16(cpu, DX);

  uint32_t dx_ax = (dx << 16) | ax;

  uint8_t op1_index = parse_reg_name(op1);
  uint16_t op1_val = get_reg16(cpu, op1_index);

  uint32_t result = dx_ax / op1_val;
  uint32_t remainder = dx_ax % op1_val;

  set_reg16(cpu, AX, (uint16_t)result);
  set_reg16(cpu, DX, (uint16_t)remainder);
}

void exec_xchg(cpu_state_t *cpu, string op1) {
  types op1_type = detect_type(op1);

  if (op1_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t ax_val = get_reg16(cpu, AX);

    set_reg16(cpu, op1_index, ax_val);
    set_reg16(cpu, AX, op1_val);
  } else {
    cpu->running = false;
  }
}
