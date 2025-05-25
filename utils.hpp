#ifndef UTILS_HPP
#define UTILS_HPP

#include "opcode.hpp"
#include <cstdint>

// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

void parse_reg_rm16(instruction_info *info, uint8_t *byte, int offset);
#endif // !UTILS_H
