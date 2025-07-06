#ifndef INTERRUPTS
#define INTERRUPTS
#include "cpu.hpp"

typedef struct {
  uint16_t process_id;
  uint16_t syscall_num;
  uint16_t m1;
  uint16_t m2;
  uint16_t m3;
  uint16_t m4;
  uint16_t m5;
  uint16_t m6;
} message_t;

void exec_int20(cpu_state_t *cpu);

#endif // !INTERRUPTS
