#ifndef SYSCALLS
#define SYSCALLS

#include "cpu.hpp"
#include <cstdint>

void sys_write(uint32_t address, int length, memory_t *memory);

#endif // !SYSCALLS
