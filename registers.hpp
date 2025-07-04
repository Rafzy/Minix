#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include "cpu.hpp"
#include <cstdint>
#include <string>
using namespace std;

extern string reg_table[][8];
extern string rm_table[];

const typedef enum {
  AX = 0,
  CX,
  DX,
  BX,
  // Special purpose registers
  SP,
  BP,
  SI,
  DI,
  // Control Registers
  CS,
  DS,
  SS,
  ES,
  IP,
  // FLAGS
  CF,
  SF,
  OF,
  ZF
} reg_names;

#endif // !REGISTERS_HPP
