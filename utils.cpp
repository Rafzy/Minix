#include "utils.hpp"
#include "opcode.hpp"
#include "registers.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

uint32_t le_32(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8 | bytes[offset + 2] << 16 |
         bytes[offset + 3] << 24;
};

uint16_t le_16(uint8_t *bytes, int offset) {
  return bytes[offset] | bytes[offset + 1] << 8;
}

void print_result(instruction_info info) {
  cout << info.mnemonic << " " << (info.op1 != "" ? info.op1 : "")
       << (info.op2 != "" ? ", " + info.op2 : "")
       << (info.op3 != "" ? ", " + info.op3 : "")
       << (info.op4 != "" ? ", " + info.op4 : "") << "\n";
}

// DEPCRECATED
// string sign_extend(int16_t val, int width = 4) {
//   std::stringstream stream;
//   stream << setfill('0') << setw(width) << std::hex << abs(val);
//   return stream.str();
// }

uint8_t parse_reg_name(const string &reg_name) {
  for (int i = 0; i < 8; i++) {
    if (reg_table[1][i] == reg_name) {
      return i;
    }
  }
  for (int i = 0; i < 8; i++) {
    if (reg_table[0][i] == reg_name) {
      return i % 4;
    }
  }
  return 255;
};

uint16_t parse_hex_string(const string &hex_str) {
  string clean_hex = hex_str;

  if (clean_hex.substr(0, 2) == "0x") {
    clean_hex = clean_hex.substr(2);
  }

  uint16_t result = 0;
  std::stringstream ss;
  ss << std::hex << clean_hex;
  ss >> result;
  return result;
}

string print_hex(int16_t val, int width) {
  std::stringstream stream;
  stream << setfill('0') << setw(width) << std::hex << val;
  return stream.str();
}
