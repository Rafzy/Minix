#ifndef UTILS_HPP
#define UTILS_HPP

#include "opcode.hpp"
#include <cstdint>

string sign_extend(int16_t val);
// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

string print_hex(int16_t val, int width = 4);

void print_result(instruction_info info);

#endif // !UTILS_H
