#include "opcode.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstdint>

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

int analyze_opcode(uint8_t *byte, int offset) {
  instruction_info result_info;
  switch (byte[offset]) {
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x56:
  case 0x57: {
    result_info.mnemonic = "push";
    uint8_t reg = byte[offset] - 0x50;
    string reg_name = reg_1[reg];
    cout << result_info.mnemonic << " ";
    cout << reg_name;
    break;
  }

  case 0x89: {
    result_info.mnemonic = "mov";
  }

  case 0xb0:
  case 0xb1:
  case 0xb2:
  case 0xb3:
  case 0xb4:
  case 0xb5:
  case 0xb6:
  case 0xb7: {
    result_info.mnemonic = "MOV";
    uint8_t reg = byte[offset] - 0xb0;
    string reg_name = reg_0[reg];
    uint16_t imm_value = byte[offset + 1];
    cout << result_info.mnemonic << '\n';
    cout << reg_name << '\n';
    cout << setfill('0') << setw(4) << hex << imm_value;
    break;
  }
  case 0xb8:
  case 0xb9:
  case 0xba:
  case 0xbb:
  case 0xbc:
  case 0xbd:
  case 0xbe:
  case 0xbf: {
    result_info.mnemonic = "mov";
    uint8_t reg = byte[offset] - 0xb8;
    string reg_name = reg_1[reg];
    uint16_t imm_value = le_16(byte, offset + 1);
    cout << result_info.mnemonic << ' ';
    cout << reg_name << ' ';
    cout << setfill('0') << setw(4) << hex << imm_value << '\n';
    break;
  }
  case 0xcd: {
    result_info.mnemonic = "int";
    uint8_t imm_value = byte[offset + 1];

    break;
  }
  }
  return 1;
}
