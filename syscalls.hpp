#ifndef SYSCALLS
#define SYSCALLS

#include "cpu.hpp"
#include <cstdint>

void sys_write(uint32_t address, int length, memory_t *memory);
void sys_ioctl(cpu_state_t *cpu);
void sys_brk(cpu_state_t *cpu, uint16_t addr, int8_t ret);

#endif // !SYSCALLS
