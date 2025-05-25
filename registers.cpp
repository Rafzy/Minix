#include "registers.hpp"
#include <string>
using namespace std;

string reg_table[][8] = {{"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"},
                         {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"}};

string rm_table[] = {"(BX) + (SI) + ", "(BX) + (DI) + ", "(BP) + (SI) + ",
                     "(BP) + (DI) + ", "(SI) + ",        "(DI) + ",
                     "(BP) + ",        "(BX) + "};
