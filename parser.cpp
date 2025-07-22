#include "parser.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstdint>

void parse_mod_reg_rm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t reg = (opcode_2 & 0x38) >> 3;
  uint8_t rm = (opcode_2 & 0x07);
  uint8_t w = (opcode & 0x01);
  string reg_name = reg_table[w][reg];
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[w][rm];
  } else if (mod == 0x01) {
    // typecast int_8 for sign extension
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    // add one when displacement uses two bytes
    info->length += 1;

  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    // add two when displacement uses two bytes
    info->length += 2;
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {
      rm_name = rm_table[rm] + "]";
    }
  }

  if ((opcode & 0x02) == 0x02) {
    info->op1 = reg_name;
    info->op2 = rm_name;
  } else {
    info->op1 = rm_name;
    info->op2 = reg_name;
  }
};

void parse_mod_reg_rm_nodw(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t reg = (opcode_2 & 0x38) >> 3;
  uint8_t rm = (opcode_2 & 0x07);
  // uint8_t w = (opcode & 0x01);
  string reg_name = reg_table[1][reg];
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[1][rm];
  } else if (mod == 0x01) {
    // typecast int_8 for sign extension
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    // add one when displacement uses two bytes
    info->length += 1;

  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    // add two when displacement uses two bytes
    info->length += 2;
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {

      rm_name = rm_table[rm] + "]";
    }
  }

  info->op1 = reg_name;
  info->op2 = rm_name;
};

void parse_mod_reg_rm_nod(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t reg = (opcode_2 & 0x38) >> 3;
  uint8_t rm = (opcode_2 & 0x07);
  uint8_t w = (opcode & 0x01);
  string reg_name = reg_table[w][reg];
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[w][rm];
  } else if (mod == 0x01) {
    // typecast int_8 for sign extension
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    // add one when displacement uses two bytes
    info->length += 1;

  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    // add two when displacement uses two bytes
    info->length += 2;
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {

      rm_name = rm_table[rm] + "]";
    }
  }

  info->op1 = reg_name;
  info->op2 = rm_name;
};

void parse_reg_end(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 1;
  uint8_t opcode = byte[offset];
  uint8_t reg = opcode - 0x50;
  info->op1 = reg_table[1][reg];
}

void parse_reg_imm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t reg = (opcode & 0x07);
  uint8_t w = (opcode & 0x08) >> 3;
  string reg_name = reg_table[w][reg];
  uint16_t imm;
  if (w == 0x01) {
    imm = le_16(byte, offset + 1);
    info->length += 1;
  } else {
    imm = byte[offset + 1];
  }
  info->op1 = reg_name;
  info->op2 = print_hex(imm, 4);
}

void parse_rm_imm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t sw = (opcode & 0x03);
  uint8_t w = (opcode & 0x01);
  uint8_t mod = (opcode_2 & 0xC0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  string rm_name;
  uint16_t data;
  if (mod == 0x03) {
    rm_name = reg_table[w][rm];
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {
      rm_name = rm_table[rm] + "]";
    }
  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp < 0 ? "-" : "+") + print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }

  if (sw == 0x01) {
    data = le_16(byte, offset + info->length);
    info->length += 2;

    string data_string;
    data_string = print_hex(data, 4);
    info->op1 = rm_name;
    info->op2 = data_string;
    return;
  }
  // TODO: handle sign extend
  else if (sw == 0x03) {
    string data_string;
    int8_t data = (int8_t)byte[offset + info->length];
    if (data < 0) {
      data_string = "-" + print_hex(abs(data), 1);
    } else {
      data_string = print_hex(abs(data), 1);
    }
    info->length += 1;
    info->op1 = rm_name;
    info->op2 = data_string;
    return;

  } else {
    data = byte[offset + info->length];
    info->length += 1;
  }

  info->op1 = rm_name;
  info->op2 = print_hex(data, 2);
}

void parse_rm_imm_no_s(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t w = (opcode & 0x01);
  uint8_t mod = (opcode_2 & 0xC0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  string rm_name;
  uint16_t data;

  if (mod == 0x03) {
    rm_name = reg_table[w][rm];
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 2) + "]";
      info->length += 2;
    } else {
      rm_name = rm_table[rm] + "]";
    }
  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp < 0 ? "-" : "+") + print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }

  if (w == 0x01) {
    data = le_16(byte, offset + info->length);
    info->length += 2;
  } else {
    data = byte[offset + info->length];
    info->length += 1;
  }

  info->op1 = rm_name;
  info->op2 = print_hex(data, 1);
}

void parse_rm(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  uint8_t w = (opcode & 0x01);
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[w][rm];
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {
      rm_name = rm_table[rm] + "]";
    }
  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp < 0 ? "-" : "+") + print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }
  info->op1 = rm_name;
}

void parse_rm_v(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  uint8_t v = (opcode & 0x02) >> 1;
  uint8_t w = (opcode & 0x01);
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[w][rm];
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {
      rm_name = rm_table[rm] + "]";
    }
  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp < 0 ? "-" : "+") + print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }
  info->op1 = rm_name;
  if (v == 0x01) {
    info->op2 = "CL";
  } else if (v == 0x00) {
    info->op2 = "1";
  }
}

void parse_dir_w_seg(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 3;
  uint16_t disp = le_16(byte, offset + 1);
  // uint16_t ea = disp + offset + info->length - 32;
  uint16_t ea = disp + offset + info->length;
  info->op1 = print_hex(ea, 4);
}

void parse_dir_with_seg(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 3;
  uint16_t disp = le_16(byte, offset + 1);
  // uint16_t ea = disp + offset + info->length - 32;
  info->op1 = print_hex(disp, 4);
}

void parse_disp(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  int8_t disp = (int8_t)byte[offset + 1];
  // uint16_t ea = (offset - 32) + disp + info->length;
  uint16_t ea = offset + disp + info->length;
  info->op1 = print_hex(ea, 4);
}

void parse_reg(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 1;
  uint8_t opcode = byte[offset];
  uint8_t reg = (opcode & 0x07);
  info->op1 = reg_table[1][reg];
};

void parse_imm_acc(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t w = (opcode & 0x01);
  uint16_t data = byte[offset + 1];
  if (w == 0x01) {
    data = le_16(byte, offset + 1);
    info->length += 1;
  }
  info->op1 = reg_table[1][0];
  // info->op2 = (data < 0 ? "-" : "") + print_hex(abs(data), 4);
  info->op2 = print_hex(data, 4);
}

void parse_indir_w_seg(instruction_info *info, uint8_t *byte, int offset) {
  info->length = 2;
  uint8_t opcode = byte[offset];
  uint8_t opcode_2 = byte[offset + 1];
  uint8_t mod = (opcode_2 & 0xc0) >> 6;
  uint8_t rm = (opcode_2 & 0x07);
  string rm_name;
  if (mod == 0x03) {
    rm_name = reg_table[1][rm];
  } else if (mod == 0x00) {
    if (rm == 0x06) {
      uint16_t disp = le_16(byte, offset + info->length);
      rm_name = "[" + print_hex(disp, 4) + "]";
      info->length += 2;
    } else {
      rm_name = rm_table[rm] + "]";
    }
  } else if (mod == 0x02) {
    int16_t disp = le_16(byte, offset + info->length);
    rm_name =
        rm_table[rm] + (disp >= 0 ? "+" : "-") + print_hex(abs(disp), 2) + "]";
    info->length += 2;
  } else if (mod == 0x01) {
    int8_t disp = (int8_t)byte[offset + info->length];
    rm_name =
        rm_table[rm] + (disp < 0 ? "-" : "+") + print_hex(abs(disp), 2) + "]";
    info->length += 1;
  }
  info->op1 = rm_name;
}
