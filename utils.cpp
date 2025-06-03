#include "utils.hpp"
#include "opcode.hpp"
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

string print_hex(int16_t val, int width) {
  std::stringstream stream;
  stream << setfill('0') << setw(width) << std::hex << val;
  return stream.str();
}
