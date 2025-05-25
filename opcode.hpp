#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

typedef enum {
  FORMAT_UNKNOWN = 0,
  FORMAT_MODRM_8,
  FORMAT_MODRM_16,
  FORMAT_REG_IMM_8,
  FORMAT_REG_IMM_16
} format_type;

typedef struct {
  string mnemonic;
  int min_length;
} instruction_info;

void analyze_opcode(uint8_t byte);
#endif // !OCODE_H
