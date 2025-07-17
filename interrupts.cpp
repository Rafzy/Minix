#include "interrupts.hpp"
#include "cpu.hpp"
#include "registers.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

void exec_int20(cpu_state_t *cpu) {
  // Handle system calls
  uint16_t virt_addr = cpu->registers[BX];

  uint8_t segment;
  (virt_addr > 0xff00) ? segment = SS : segment = DS;

  uint16_t data_segment = cpu->registers[segment];
  message_t msg;

  uint32_t addr = (data_segment << 4) + virt_addr;
  msg.process_id = le_16(cpu->memory->data, addr);
  msg.syscall_num = le_16(cpu->memory->data, addr + 2);
  msg.m1 = le_16(cpu->memory->data, addr + 4);
  msg.m2 = le_16(cpu->memory->data, addr + 6);
  msg.m3 = le_16(cpu->memory->data, addr + 8);
  msg.m4 = le_16(cpu->memory->data, addr + 10);
  msg.m5 = le_16(cpu->memory->data, addr + 12);
  msg.m6 = le_16(cpu->memory->data, addr + 14);

  cpu->registers[AX] = 0;

  // Syscall cases
  switch (msg.syscall_num) {
  case 1: {
    // TODO:
    // Do the syscall exit
    // printf("EXIT\n");
    cpu->running = false;
  }
  case 4: {
    // printf("%04x %04x %04x %04x %04x %04x %04x %04x\n", msg.process_id,
    //        msg.syscall_num, msg.m1, msg.m2, msg.m3, msg.m4, msg.m5, msg.m6);

    uint8_t segment;
    msg.m4 > 0xff00 ? segment = SS : segment = DS;
    uint32_t write_addr = (cpu->registers[segment] << 4) + msg.m4;
    sys_write(write_addr, msg.m2, cpu->memory);
    break;
  }
  default: {
    printf("ERROR: SYSCALL NOT FOUND\n");
    break;
  }
  }
}
