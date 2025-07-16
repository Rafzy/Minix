#include "utils.hpp"
#include "cpu.hpp"
#include "opcode.hpp"
#include "registers.hpp"
#include <cstdint>
#include <cstring>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

uint32_t le_32(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8 | bytes[offset + 2] << 16 |
         bytes[offset + 3] << 24;
};

uint16_t le_16(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8;
}

// Dissassembler function
void print_result(instruction_info info) {
  cout << info.mnemonic << " " << (info.op1 != "" ? info.op1 : "")
       << (info.op2 != "" ? ", " + info.op2 : "")
       << (info.op3 != "" ? ", " + info.op3 : "")
       << (info.op4 != "" ? ", " + info.op4 : "") << "\n";
}

// DEPCRECATED
// string sign_extend(int16_t val, int width = 4) {
//   std::stringstream stream;
//   stream << setfill('0') << setw(width) << std::hex << abs(val);
//   return stream.str();
// }

uint8_t parse_reg_name(const string &reg_name) {
  for (int i = 0; i < 8; i++) {
    if (reg_table[1][i] == reg_name) {
      return i;
    }
  }
  for (int i = 0; i < 8; i++) {
    if (reg_table[0][i] == reg_name) {
      return i % 4;
    }
  }
  return 255;
};

uint16_t parse_memory(cpu_state_t *cpu, const string &mem_name) {
  string clean_mem = mem_name;

  int len = clean_mem.size();
  clean_mem = clean_mem.substr(1, len - 2);

  int result = 0;
  int sign = 1;
  string token = "";

  // clean_mem = "+" + clean_mem;
  clean_mem = clean_mem + "+";

  for (size_t i = 0; i < len; ++i) {
    char c = clean_mem[i];

    if (c == '+' | c == '-') {
      if (!token.empty()) {
        size_t start = token.find_first_not_of(" \t");
        size_t end = token.find_last_not_of(" \t");

        if (start != string::npos && end != string::npos) {
          token = token.substr(start, end - start + 1);

          if (isdigit(token[0])) {
            try {
              // int imm_val = stoi(token);
              int imm_val = parse_hex_string(token);
              result += sign * imm_val;
            } catch (const exception &e) {
              throw invalid_argument("Invalid imm value: " + token);
            }
          } else {
            try {
              uint8_t reg_index = parse_reg_name(token);
              result += sign * cpu->registers[reg_index];
            } catch (const exception &e) {
              throw invalid_argument("Invalid register name: " + token);
            }
          }
        }
      }
      sign = (c == '+') ? 1 : -1;
      token = "";
    } else {
      token += c;
    }
  }

  return result;
}

// Turns out there's already a function called stoi lol
uint16_t parse_hex_string(const string &hex_str) {
  string clean_hex = hex_str;

  if (clean_hex.substr(0, 2) == "0x") {
    clean_hex = clean_hex.substr(2);
  }

  uint16_t result = 0;
  std::stringstream ss;
  ss << std::hex << clean_hex;
  ss >> result;
  return result;
}

string print_hex(int16_t val, int width) {
  std::stringstream stream;
  stream << setfill('0') << setw(width) << std::hex << val;
  return stream.str();
}

void set_mem(cpu_state_t *cpu, uint16_t segment, uint16_t offset,
             uint8_t value) {
  uint32_t physical_addr = (segment << 4) + offset;
  cpu->memory->data[physical_addr] = value;
}

void set_mem16(cpu_state_t *cpu, uint16_t segment, uint16_t offset,
               uint16_t value) {
  uint32_t physical_addr = (segment << 4) + offset;
  cpu->memory->data[physical_addr] = value & 0xFF;
  cpu->memory->data[physical_addr + 1] = value >> 8;
}

void push_stack(cpu_state_t *cpu, uint16_t value) {
  cpu->registers[SP] -= 2;
  set_mem16(cpu, cpu->registers[SS], cpu->registers[SP], value);
}

uint16_t pop_stack(cpu_state_t *cpu) {
  uint16_t pop_val = get_mem_16(cpu, cpu->registers[SS], cpu->registers[SP]);
  cpu->registers[SP] += 2;
  return pop_val;
}

uint16_t get_mem_16(cpu_state_t *cpu, uint16_t segment, uint16_t offset) {
  uint32_t physical_addr = (segment << 4) + offset;
  return le_16(cpu->memory->data, physical_addr);
}

// Debugging functions
void print_cpu_state_header() {
  printf("AX   BX   CX   DX   SP   BP   SI   DI   FLAGS IP\n");
}

void print_flags(cpu_state_t *cpu) {
  printf("%c%c%c%c",
         cpu->registers[OF] ? 'O' : '-', // Overflow
         cpu->registers[SF] ? 'S' : '-', // Sign
         cpu->registers[ZF] ? 'Z' : '-', // Zero
         cpu->registers[CF] ? 'C' : '-'  // Carry
  );
}

void print_cpu_log(cpu_state_t *cpu) {
  static int first_call = 1;

  if (first_call) {
    print_cpu_state_header();
    first_call = 0;
  }

  printf("%04x %04x %04x %04x %04x %04x %04x %04x ", cpu->registers[AX],
         cpu->registers[BX], cpu->registers[CX], cpu->registers[DX],
         cpu->registers[SP], cpu->registers[BP], cpu->registers[SI],
         cpu->registers[DI]);

  print_flags(cpu);
  printf(" ");

  printf("%04x:%04x", cpu->registers[IP], 0x0000);

  printf("    ");
  // printf("\n");
}
