#include "utils.hpp"
#include "opcode.hpp"
#include "registers.hpp"
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

uint32_t le_32(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8 | bytes[offset + 2] << 16 |
         bytes[offset + 3] << 24;
};

uint16_t le_16(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8;
}

string sign_extend(int16_t val) {
  std::stringstream stream;
  stream << std::hex << abs(val);
  return stream.str();
}

void parse_reg_rm16(instruction_info *info, uint8_t *byte, int offset) {
  uint8_t opcode = byte[offset];
  uint8_t operand = byte[offset + 1];
  uint8_t mod = (operand & 0xc0) >> 6;
  uint8_t reg = (operand & 0x38) >> 3;
  uint8_t rm = (operand & 0x07);
  string reg_name = reg_table[1][reg];
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[1][rm];
  } else if (mod == 0x01) {
    // typecast int_8 for sign extension
    int8_t disp = (int8_t)byte[offset + 2];
    string se_disp = sign_extend(disp);
    rm_name = rm_table[rm] + (disp >= 0 ? " + " : " - ") + se_disp;
  } else if (mod == 0x02) {
    int16_t disp = (int8_t)le_16(byte, offset + 2);
    string rm_name = rm_table[rm] + " + " + to_string(disp);
  } else if (mod == 0x00) {
    rm_name = rm_table[rm];
  }

  if ((opcode & 0x02) == 0x02) {
    info->op1 = reg_name;
    info->op2 = rm_name;
  } else {
    info->op1 = rm_name;
    info->op2 = reg_name;
  }
};
