#include "utils.hpp"
#include "opcode.hpp"
#include "registers.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

uint32_t le_32(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8 | bytes[offset + 2] << 16 |
         bytes[offset + 3] << 24;
};

uint16_t le_16(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8;
}

// DEPCRECATED
// string sign_extend(int16_t val, int width = 4) {
//   std::stringstream stream;
//   stream << setfill('0') << setw(width) << std::hex << abs(val);
//   return stream.str();
// }

string print_hex(int16_t val, int width = 4) {
  std::stringstream stream;
  stream << setfill('0') << setw(width) << std::hex << val;
  return stream.str();
}

// Opcodes:
// MOV
void parse_mod_reg_rm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t reg = (opcode_2 & 0x38) >> 3;
  uint8_t rm = (opcode_2 & 0x07);
  uint8_t w = (opcode & 0x01);
  string reg_name = reg_table[w][reg];
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[1][rm];
  } else if (mod == 0x01) {
    // typecast int_8 for sign extension
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name = rm_table[rm] + (disp >= 0 ? " + " : " - ") +
              print_hex(abs(disp), 2) + "]";
    // add one when displacement uses two bytes
    info->length += 1;

  } else if (mod == 0x02) {
    uint16_t disp = le_16(byte, offset + info->length);
    rm_name = rm_table[rm] + " + " + print_hex(disp, 2) + "]";
    // add two when displacement uses two bytes
    info->length += 2;
  } else if (mod == 0x00) {
    rm_name = rm_table[rm] + "]";
  }

  if ((opcode & 0x02) == 0x02) {
    info->op1 = reg_name;
    info->op2 = rm_name;
  } else {
    info->op1 = rm_name;
    info->op2 = reg_name;
  }
};

// Opcodes:
// PUSH
void parse_reg_end(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 1;
  uint8_t opcode = byte[offset];
  uint8_t reg = opcode - 0x50;
  info->op1 = reg_table[1][reg];
}

// Opcodes:
// MOV
void parse_reg_imm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t reg = (opcode & 0x07);
  uint8_t w = (opcode & 0x08) >> 3;
  string reg_name = reg_table[w][reg];
  uint16_t imm;
  if (w == 0x01) {
    imm = le_16(byte, offset + 1);
    info->length += 1;
  } else {
    imm = byte[offset + 1];
  }
  info->op1 = reg_name;
  info->op2 = print_hex(imm, 4);
}

// Opcodes:
// ADD
// ADC
void parse_rm_imm(instruction_info *info, uint8_t *byte, int offset) {
  // TODO: Finish this shit
  info->length = 3;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t sw = (opcode & 0x03);
  uint8_t mod = (opcode_2 & 0xC0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  string rm_name;
  uint16_t data;
  if (sw == 0x01) {
    // Use second data byte
    info->length += 1;
    data = le_16(byte, offset + 2);

  }
  // TODO: handle sign extend
  // else if (sw == 0x03) {
  //   int8_t data = (int8_t)byte[offset + 2];
  // }
  else {
    data = byte[offset + 2];
  }
  if (mod == 0x03) {
    rm_name = reg_table[1][rm];
  } else if (mod == 0x00) {
    rm_name = rm_table[rm] + "]";
  } else if (mod == 0x02) {
    uint16_t disp = le_16(byte, offset + info->length);
    rm_name = rm_table[rm] + " + " + print_hex(disp, 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name = rm_table[rm] + (disp < 0 ? " - " : " + ") +
              print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }

  info->op1 = rm_name;
  info->op2 = print_hex(data, 1);
}

// Opcodes:
// PUSH
void parse_rm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  int8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[1][rm];
  } else if (mod == 0x00) {
    rm_name = rm_table[rm] + "]";
  } else if (mod == 0x02) {
    uint16_t disp = le_16(byte, offset + info->length);
    rm_name = rm_table[rm] + " + " + print_hex(disp, 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name = rm_table[rm] + (disp < 0 ? " - " : " + ") +
              print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }
  info->op1 = rm_name;
}

// Opcodes:
// CALL
// JMP
void parse_dir_w_seg(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 3;
  int16_t disp = (int16_t)le_16(byte, offset + 1);
  uint16_t ea = disp + offset + info->length - 32;
  info->op1 = print_hex(ea, 4);
}

// Opcodes:
// JMP SHORT
void parse_dir_w_seg_short(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  int16_t disp = (int16_t)byte[offset + 1];
  uint16_t ea = disp + offset + info->length - 32;
  info->op1 = print_hex(ea, 4);
}

// Opcodes:
// POP
// PUSH
void parse_reg(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 1;
  uint8_t opcode = byte[offset];
  uint8_t reg = (opcode & 0x07);
  info->op1 = reg_table[1][reg];
};
