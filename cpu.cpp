#include "cpu.hpp"
#include "interrupts.hpp"
#include "parser.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstring>
#include <iostream>

types detect_type(const string &name) {
  if (name[0] == '[' && name.back() == ']') {
    return MEMORY;
  } else if (name.find_first_not_of("0123456789ABCDEFabcdef") == string::npos) {
    return IMMEDIATE;
  } else {
    return REGISTER;
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
  cpu->registers[SS] = 0x3000;
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

void set_reg16(cpu_state_t *cpu, uint8_t reg, uint16_t val) {
  if (reg > 7)
    return;
  cpu->registers[reg] = val;
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
    exec_ret(cpu);
  } else if (info.mnemonic == "or") {
    exec_or(cpu, info.op1, info.op2);
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

  if (dst_type == REGISTER && src_type == REGISTER) {
    uint8_t reg_src = parse_reg_name(src);
    uint8_t reg_dst = parse_reg_name(dst);
    uint16_t reg_src_val = get_reg16(cpu, reg_src);

    set_reg16(cpu, reg_dst, reg_src_val);
  }

  if (dst_type == REGISTER && src_type == MEMORY) {
    uint8_t reg_dst = parse_reg_name(dst);
    uint16_t mem_addr = parse_memory(cpu, src);
    uint32_t full_addr = (cpu->registers[SS] << 4) + mem_addr;
    uint16_t mem_val = le_16(cpu->memory->data, full_addr);

    set_reg16(cpu, reg_dst, mem_val);
  }

  if (dst_type == MEMORY && src_type == REGISTER) {
    uint8_t reg_src = parse_reg_name(src);
    uint16_t mem_addr = parse_memory(cpu, dst);
    uint16_t src_val = get_reg16(cpu, reg_src);

    set_mem(cpu, cpu->registers[SS], mem_addr, src_val & 0xff);
    set_mem(cpu, cpu->registers[SS], mem_addr + 1, src_val >> 8);
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
  }
  if (op1_type == REGISTER && op2_type == MEMORY) {
    // TODO:
  }
}

void exec_or(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint8_t op1_index = parse_reg_name(op1);
    uint8_t op2_index = parse_reg_name(op2);

    uint16_t result = cpu->registers[op1_index] | cpu->registers[op2_index];
    result == 0 ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
    (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
  }
}

void exec_lea(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == MEMORY) {
    uint16_t reg_index = parse_reg_name(op1);
    uint16_t mem_addr = parse_memory(cpu, op2);

    set_reg16(cpu, reg_index, mem_addr);
  }
}

void exec_add(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint16_t dst_index = parse_reg_name(op1);
    uint16_t src_index = parse_reg_name(op2);
    uint16_t total = cpu->registers[src_index] + cpu->registers[dst_index];

    // Update Z Flag
    (total == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
    // Update S Flag
    ((total >> 15) == 1) ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;

    set_reg16(cpu, dst_index, total);
  }
}

void exec_sub(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint8_t op1_index = parse_reg_name(op1);
    uint16_t op1_val = get_reg16(cpu, op1_index);
    uint16_t op2_val = parse_hex_string(op2);

    uint16_t result = op1_val - op2_val;

    // Update Z Flag
    (result == 0) ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
    // Update S Flag
    ((result >> 15) == 1) ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;

    cpu->registers[op1_index] = result;
  }
}

void exec_cmp(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == REGISTER) {
    uint16_t eax_index = parse_reg_name(op1);
    uint16_t ebx_index = parse_reg_name(op2);

    if (get_reg16(cpu, eax_index) < get_reg16(cpu, ebx_index)) {
      cpu->registers[CF] = 1;
    } else if (get_reg16(cpu, eax_index) >= get_reg16(cpu, ebx_index)) {
      cpu->registers[CF] = 0;
    }

    if (cpu->registers[eax_index] - cpu->registers[ebx_index] == 0) {
      cpu->registers[ZF] = 1;
    } else {
      cpu->registers[ZF] = 0;
    }
  }

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint16_t op1_index = parse_reg_name(op1);
    uint16_t op2_val = parse_hex_string(op2);

    if (get_reg16(cpu, op1_index) < op2_val) {
      cpu->registers[CF] = 1;
    } else if (get_reg16(cpu, op1_index) >= op2_val) {
      cpu->registers[CF] = 0;
    }
    if (cpu->registers[op1_index] - op2_val == 0) {
      cpu->registers[ZF] = 1;
    } else {
      cpu->registers[ZF] = 0;
    }
  }

  if (op1_type == MEMORY && op2_type == IMMEDIATE) {
    uint16_t mem_op1 = parse_memory(cpu, op1);
    uint16_t op2_val = parse_hex_string(op2);
    uint32_t full_addr = (cpu->registers[DS] << 4) + mem_op1;
    uint16_t op1_val = le_16(cpu->memory->data, full_addr);

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
  }
}

void exec_jnb(cpu_state_t *cpu, string op1) {
  uint16_t op_val = parse_hex_string(op1);

  if (cpu->registers[CF] == 0) {
    cpu->registers[IP] += op_val;
  }
}

void exec_test(cpu_state_t *cpu, string op1, string op2) {
  types op1_type = detect_type(op1);
  types op2_type = detect_type(op2);

  if (op1_type == REGISTER && op2_type == IMMEDIATE) {
    uint16_t op1_index = parse_reg_name(op1);
    uint16_t op2_index = parse_reg_name(op2);
    uint16_t result = get_reg16(cpu, op1_index) & get_reg16(cpu, op2_index);
    // Update Flags
    cpu->registers[CF] = 0;
    result == 0 ? cpu->registers[ZF] = 1 : cpu->registers[ZF] = 0;
    // Update S Flag
    (result >> 15) == 1 ? cpu->registers[SF] = 1 : cpu->registers[SF] = 0;
  }
}

void exec_jne(cpu_state_t *cpu, string op1) {
  uint16_t op1_val = parse_hex_string(op1);

  if (cpu->registers[ZF] == 0) {
    cpu->registers[IP] += op1_val;
  }
}

void exec_je(cpu_state_t *cpu, string op1) {
  uint16_t op1_val = parse_hex_string(op1);

  if (cpu->registers[ZF] == 1) {
    cpu->registers[IP] = op1_val;
    cpu->registers[IP] -= 2;
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
    uint16_t op1_val = get_mem_16(cpu, cpu->registers[SS], mem_addr);
    push_stack(cpu, op1_val);
  }
}

void exec_call(cpu_state_t *cpu, string op1) {
  uint16_t op1_val = parse_hex_string(op1);
  cpu->registers[IP] += 3;
  push_stack(cpu, cpu->registers[IP]);
  cpu->registers[IP] = op1_val;

  // This is stupid
  cpu->registers[IP] -= 3;
}

void exec_jmp(cpu_state_t *cpu, string op1) {
  uint16_t op1_val = parse_hex_string(op1);

  cpu->registers[IP] = op1_val;
  cpu->registers[IP] -= 3;
}

void exec_jmp_short(cpu_state_t *cpu, string op1) {

  uint16_t op1_val = parse_hex_string(op1);

  cpu->registers[IP] = op1_val;
  cpu->registers[IP] -= 2;
}

void exec_pop(cpu_state_t *cpu, string op1) {
  uint8_t op1_index = parse_reg_name(op1);
  cpu->registers[op1_index] = pop_stack(cpu);
}

void exec_ret(cpu_state_t *cpu) {
  cpu->registers[IP] = pop_stack(cpu);
  cpu->registers[IP] -= 1;
}
