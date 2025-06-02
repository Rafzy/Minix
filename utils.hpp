#ifndef UTILS_HPP
#define UTILS_HPP

#include "opcode.hpp"
#include <cstdint>
string sign_extend(int16_t val);
// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

string print_hex(int16_t val, int width = 4);

// XXXXXXdw mod|reg|r/m
void parse_mod_reg_rm(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXX mod|reg|r/m
void parse_mod_reg_rm_nodw(instruction_info *info, uint8_t *byte, int offset);

// XXXXXreg
void parse_reg_end(instruction_info *info, uint8_t *byte, int offset);

// XXXXw|reg data data(if w = 1)
void parse_reg_imm(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXsw mod|000|rm data data(if s:w = 01)
void parse_rm_imm(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXw mod|000|rm data data(if w = 1)
void parse_rm_imm_no_s(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXX disp-low disp-high
void parse_dir_w_seg(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXX disp
void parse_disp(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXw mod|XXX|rm
void parse_rm(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXvw mod|XXX|rm
void parse_rm_v(instruction_info *info, uint8_t *byte, int offset);

// XXXXX|REG
void parse_reg(instruction_info *info, uint8_t *byte, int offset);

// XXXXXXXw data data(if w = 1)
void parse_imm_acc(instruction_info *info, uint8_t *byte, int offset);
#endif // !UTILS_H
