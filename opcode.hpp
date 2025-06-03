#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <cstdint>
#include <string>

using namespace std;

typedef struct {
  string mnemonic;
  string op1;
  string op2;
  string op3;
  string op4;
  int length;
} instruction_info;

instruction_info analyze_opcode(uint8_t *byte, int *offset);
#endif // !OPCODE_H
