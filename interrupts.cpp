#include "interrupts.hpp"
#include "cpu.hpp"
#include "registers.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

void exec_int20(cpu_state_t *cpu) {
  // Handle system calls
  uint16_t virt_addr = cpu->registers[BX];
  uint16_t data_segment = cpu->registers[DS];
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

  // Syscall cases
  switch (msg.syscall_num) {
  case 1: {
    // TODO:
    // Do the syscall exit
    printf("EXIT\n");
  }
  case 4: {
    uint32_t write_addr = (data_segment << 4) + msg.m4;
    sys_write(write_addr, msg.m2, cpu->memory);
    break;
  }
  default: {
    printf("ERROR: SYSCALL NOT FOUND");
    break;
  }
  }
}
