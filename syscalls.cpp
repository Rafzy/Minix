#include "syscalls.hpp"
#include "cpu.hpp"
#include "registers.hpp"
#include "syscall.h"
#include "utils.hpp"
#include <iostream>
#include <stdio.h>

#define WRITE 4

void sys_write(uint32_t address, int length, memory_t *memory) {
  for (uint32_t i = address; i < address + length; i++) {
    if (memory->data[i] == '\n') {
      fprintf(stdout, "\n");
    } else if (memory->data[i] == '\r') {
      // Skip it or handle it appropriately
      // You might want to check if next char is \n for \r\n sequences
    } else {
      fprintf(stdout, "%c", memory->data[i]);
    }
  }
  fflush(stdout);
}

void sys_ioctl(cpu_state_t *cpu) {
  uint16_t addr = cpu->registers[BX] + 2;
  uint8_t segment;
  addr > 0xf000 ? segment = SS : segment = DS;

  set_mem16(cpu, cpu->registers[segment], addr, 0xffea);
}

void sys_brk(cpu_state_t *cpu, uint16_t addr, int8_t ret) {
  uint16_t dst = cpu->registers[BX];
  uint8_t segment;
  (dst > 0xf000) ? segment = SS : segment = DS;

  set_mem16(cpu, cpu->registers[segment], dst + 2, ret);
  set_mem16(cpu, cpu->registers[segment], dst + 18, addr);
}
