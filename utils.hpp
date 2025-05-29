#ifndef UTILS_HPP
#define UTILS_HPP

#include "opcode.hpp"
#include <cstdint>
string sign_extend(int16_t val);
// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

string print_hex(int16_t val);

// XXXXXXdw mod|reg|r/m
void parse_mod_reg_rm(instruction_info *info, uint8_t *byte, int offset);

// XXXXXreg
void parse_reg_end(instruction_info *info, uint8_t *byte, int offset);

// XXXXw|reg data data(if w = 1)
void parse_reg_imm(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXX disp-low disp-high
void parse_dir_w_seg(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXX disp
void parse_dir_w_seg_short(instruction_info *info, uint8_t *byte, int offset);
#endif // !UTILS_H
