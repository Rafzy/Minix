#ifndef UTILS_HPP
#define UTILS_HPP

#include "cpu.hpp"
#include "opcode.hpp"
#include <cstdint>
#include <string>

string sign_extend(int16_t val);
// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

// Parse string because i stupid code
uint8_t parse_reg_name(const string &reg_name);
uint16_t parse_memory(cpu_state_t *cpu, const string &mem_name);

uint16_t parse_hex_string(const string &hex_str);

string print_hex(int16_t val, int width = 4);

void print_result(instruction_info info);

#endif // !UTILS_H
