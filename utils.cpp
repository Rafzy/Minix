#include "utils.hpp"
#include "opcode.hpp"
#include "registers.hpp"
#include <cstdint>

uint32_t le_32(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8 | bytes[offset + 2] << 16 |
         bytes[offset + 3] << 24;
};

uint16_t le_16(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8;
}

void parse_reg_rm16(instruction_info *info, uint8_t *byte, int offset) {
  uint8_t opcode = byte[offset];
  uint8_t operand = byte[offset + 1];
  uint8_t mod = (operand & 0xc0) >> 6;
  uint8_t reg = (operand & 0x38) >> 3;
  uint8_t rm = (operand & 0x07);
  if (mod == 0x03) {
    string rm_name = reg_table[1][rm];
    string reg_name = reg_table[1][reg];
  }
  if (mod == 0x01) {
    int8_t disp = byte[offset + 2];
    string reg_name = reg_table[1][reg];
    string rm_name = rm_table[rm] + (char)('0' + (char)disp);
  }
};
