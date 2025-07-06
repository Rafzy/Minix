#include "syscalls.hpp"
#include "cpu.hpp"
#include "syscall.h"
#include <iostream>

#define WRITE 4

void sys_write(uint32_t address, int length, memory_t *memory) {
  for (uint32_t i = address; i < address + 6; i++) {
    printf("%c", memory->data[i]);
  }
}
