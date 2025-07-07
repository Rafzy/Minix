#include "registers.hpp"
#include "cpu.hpp"
#include <cstdint>
#include <string>
using namespace std;

string reg_table[][8] = {{"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"},
                         {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"}};

string rm_table[] = {"[BX+SI", "[BX+DI", "[BP+SI", "[BP+DI",
                     "[SI",    "[DI",    "[BP",    "[BX"};
