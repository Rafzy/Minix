#include "opcode.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstdint>

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

void print_op_byte(uint8_t *byte, int offset, int length) {
  for (int i = 0; i < length; i++) {
    cout << print_hex(byte[offset + i], 2);
  }
  cout << setfill(' ') << setw(8) << " ";
}

void print_result(instruction_info info) {
  cout << info.mnemonic << " " << (info.op1 != "" ? info.op1 : "")
       << (info.op2 != "" ? ", " + info.op2 : "")
       << (info.op3 != "" ? ", " + info.op3 : "")
       << (info.op4 != "" ? ", " + info.op4 : "");
}

int analyze_opcode(uint8_t *byte, int *offset) {
  instruction_info result_info;
  result_info.mnemonic = "[UNKNOWN]";
  result_info.op1 = "";
  result_info.op2 = "";
  result_info.op3 = "";
  result_info.op4 = "";
  result_info.length = 1;

  switch (byte[*offset]) {

  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03: {
    result_info.mnemonic = "add";
    parse_mod_reg_rm(&result_info, byte, *offset);
    break;
  }

  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b: {
    result_info.mnemonic = "or";
    parse_mod_reg_rm(&result_info, byte, *offset);
    break;
  }
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b: {
    result_info.mnemonic = "sbb";
    parse_mod_reg_rm(&result_info, byte, *offset);
    break;
  }
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
    break;
  }

  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
  case 0x5c:
  case 0x5d:
  case 0x5e:
  case 0x5f: {
    result_info.mnemonic = "pop";
    parse_reg(&result_info, byte, *offset);
    break;
  }
  case 0x74: {
    result_info.mnemonic = "je";
    parse_disp(&result_info, byte, *offset);
    break;
  }
  case 0x7d: {
    result_info.mnemonic = "jnl";
    parse_disp(&result_info, byte, *offset);
    break;
  }
  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83: {
    switch ((byte[(*offset) + 1] & 0x38) >> 3) {
    case 0x00: {
      // ADD Immediate to register/memory
      result_info.mnemonic = "add";
      parse_rm_imm(&result_info, byte, *offset);
      break;
    }
    case 0x07: {
      result_info.mnemonic = "cmp";
      parse_rm_imm(&result_info, byte, *offset);
    }
    }
    break;
  }

  case 0x88:
  case 0x89:
  case 0x8A:
  case 0x8B: {
    result_info.mnemonic = "mov";
    parse_mod_reg_rm(&result_info, byte, *offset);
    break;
  }

  case 0x8d: {
    // TODO: Fix this
    result_info.mnemonic = "lea";
    parse_mod_reg_rm_nodw(&result_info, byte, *offset);
    break;
  }
  case 0x98: {
    result_info.mnemonic = "cbw";
    result_info.length = 1;
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
    break;
  }
  case 0xc3: {
    result_info.mnemonic = "ret";
    result_info.length = 1;
    break;
  }
  case 0xcd: {
    result_info.mnemonic = "int";
    result_info.length = 2;
    uint8_t val = byte[(*offset) + 1];
    result_info.op1 = print_hex(val, 2);
    break;
  }
  case 0xe4:
  case 0xe5: {
    result_info.mnemonic = "in";
    result_info.length = 2;
    uint8_t opcode = byte[*offset];
    uint8_t port = byte[(*offset) + 1];
    uint8_t w = opcode & 0x01;
    result_info.op1 = reg_table[w][0];
    result_info.op2 = print_hex(port, 2);
    break;
  }
  case 0xec:
  case 0xed: {
    result_info.mnemonic = "in";
    result_info.length = 1;
    uint8_t opcode = byte[*offset];
    uint8_t w = opcode & 0x01;
    result_info.op1 = reg_table[w][0];
    result_info.op2 = reg_table[1][2];
    break;
  }
  case 0xe8: {
    result_info.mnemonic = "call";
    parse_dir_w_seg(&result_info, byte, *offset);
    break;
  }
  case 0xe9: {
    result_info.mnemonic = "jmp";
    parse_dir_w_seg(&result_info, byte, *offset);
    break;
  }
  case 0xeb: {
    result_info.mnemonic = "jmp short";
    parse_disp(&result_info, byte, *offset);
    break;
  }
  case 0xf6:
  case 0xf7: {
    result_info.mnemonic = "neg";
    parse_rm(&result_info, byte, *offset);
    break;
  }
  case 0xff: {
    result_info.mnemonic = "push";
    parse_rm(&result_info, byte, *offset);
    break;
  }
  }

  // TODO: Make print better
  // print_op_byte(byte, *offset, result_info.length);
  print_result(result_info);

  *offset += result_info.length;
  return 1;
}
