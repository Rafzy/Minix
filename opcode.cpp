#include "opcode.hpp"
#include <cstdint>

#include <iostream>
#include <string>

using namespace std;

void analyze_opcode(uint8_t byte) {
  instruction_info result_info;
  if (byte >= 0xb0 && byte <= 0xb7) {
    result_info.mnemonic = "MOV";
    result_info.format = FORMAT_REG_IMM_8;
    result_info.min_length = 2;

    cout << "Big Bruh";
  }
}
