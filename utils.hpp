#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>

// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

void parser_imm8(uint8_t *bytes, int offset);

#endif // !UTILS_H
