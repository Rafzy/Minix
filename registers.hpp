#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <cstdint>
#include <string>
using namespace std;

extern string reg_table[][8];
extern string rm_table[];

typedef enum { AX = 0, BX, CX, DX, SP, BP, SI, DI } reg_names;
extern uint16_t reg_vals[8];
#endif // !REGISTERS_HPP
