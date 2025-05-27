#include "opcode.hpp"
#include "utils.hpp"
#include <cstdint>

#include <iostream>
#include <string>

using namespace std;

void print_result(instruction_info info) {
  // TODO:
}

int analyze_opcode(uint8_t *byte, int *offset) {
  instruction_info result_info;
  switch (byte[*offset]) {
  case 0x50:
  case 0x51:
  case 0x52:
  case 0x53:
  case 0x54:
  case 0x55:
  case 0x56:
  case 0x57: {
    result_info.mnemonic = "push";
    parse_reg_end(&result_info, byte, *offset);
    cout << result_info.mnemonic << " ";
    cout << result_info.op1 << "\n";
    break;
  }

  case 0x88:
  case 0x89:
  case 0x8A:
  case 0x8B: {
    result_info.mnemonic = "mov";
    parse_mod_reg_rm(&result_info, byte, *offset);
    cout << result_info.mnemonic << " " << result_info.op1 << ", "
         << result_info.op2 << "\n";
    break;
  }

  case 0xb0:
  case 0xb1:
  case 0xb2:
  case 0xb3:
  case 0xb4:
  case 0xb5:
  case 0xb6:
  case 0xb7:
  case 0xb8:
  case 0xb9:
  case 0xba:
  case 0xbb:
  case 0xbc:
  case 0xbd:
  case 0xbe:
  case 0xbf: {
    result_info.mnemonic = "mov";
    parse_reg_imm(&result_info, byte, *offset);
    cout << result_info.mnemonic << " ";
    cout << result_info.op1 << ", ";
    cout << result_info.op2 << '\n';
    break;
  }
  case 0xcd: {
    result_info.mnemonic = "int";
    result_info.length = 2;
    uint8_t imm_value = byte[*offset + 1];
    cout << result_info.mnemonic << " " << hex << (int)imm_value << "\n";
    break;
  }
  case 0xe8: {
    result_info.mnemonic = "call";
    parse_dir_w_seg(&result_info, byte, *offset);
    cout << result_info.mnemonic << " " << result_info.op1 << "\n";
  }

  default: {
    result_info.length = 1;
  }
  }

  *offset += result_info.length;
  return 1;
}
