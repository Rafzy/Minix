#include "interrupts.hpp"
#include "cpu.hpp"
#include "registers.hpp"
#include "syscalls.hpp"
#include "utils.hpp"

void exec_int20(cpu_state_t *cpu) {
  // Handle system calls
  uint16_t virt_addr = cpu->registers[BX];

  uint8_t segment;
  (virt_addr > 0xf000) ? segment = SS : segment = DS;

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

  printf("MESSAGE: %04x %04x %04x %04x %04x %04x %04x %04x\n", msg.process_id,
         msg.syscall_num, msg.m1, msg.m2, msg.m3, msg.m4, msg.m5, msg.m6);
  // Syscall cases
  switch (msg.syscall_num) {
  case 0x01: {
    // TODO:
    // Do the syscall exit
    // printf("EXIT\n");
    cpu->running = false;
    break;
  }
  case 0x04: {
    uint8_t segment;
    msg.m4 > 0xf000 ? segment = SS : segment = DS;
    uint32_t write_addr = (cpu->registers[segment] << 4) + msg.m4;
    sys_write(write_addr, msg.m2, cpu->memory);
    (cpu->registers[BX] + 2 > 0xf000) ? segment = SS : segment = DS;
    set_mem16(cpu, cpu->registers[segment], cpu->registers[BX] + 2, msg.m2);
    break;
  }
  case 0x11: {
    printf("<EXECUTED BRK>\n");
    sys_brk(cpu, msg.m4, 0);
    break;
  }
  case 0x36: {
    printf("<EXECUTED IOCTL>\n");
    sys_ioctl(cpu);
    break;
  }
  default: {
    printf("ERROR: SYSCALL NOT FOUND\n");
    break;
  }
  }
}
