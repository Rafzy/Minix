#include "utils.hpp"
#include "opcode.hpp"
#include <cstdint>

uint32_t le_32(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8 | bytes[offset + 2] << 16 |
         bytes[offset + 3] << 24;
};

uint16_t le_16(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8;
}

void parser_imm8(uint8_t *bytes, int offset) {}
