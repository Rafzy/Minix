#include "cpu.hpp"
#include "opcode.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

using namespace std;

typedef struct {
  uint16_t a_magic;
  uint8_t a_flags;
  uint8_t a_cpu;
  uint8_t a_hdrlen;
  uint8_t a_unused;
  uint8_t a_version;
  uint32_t a_text;
  uint32_t a_data;
  uint32_t a_bss;
  uint32_t a_entry;
  uint32_t a_total;
  uint32_t a_syms;
} header;

int main(int argc, char *argv[]) {
  FILE *file;
  uint8_t *buffer;
  long byte_size;

  if (argc < 2) {
    printf("Argument format wrong");
    return 1;
  }

  file = fopen(argv[1], "rb");
  if (!file) {
    perror("Reading File Failed");
    return 1;
  }

  for (int i = 0; i < argc; i++) {
    // TODO: Handle parameters
  }

  // change pointer to the end of the file
  fseek(file, 0, SEEK_END);
  // return the current position of pointer from the beginning of the file
  byte_size = ftell(file);
  // Go back to the start of the file
  fseek(file, 0, SEEK_SET);

  buffer = (uint8_t *)malloc(byte_size);

  if (!buffer) {
    perror("Allocating memory failed");
    fclose(file);
    return 1;
  }

  if (fread(buffer, 1, byte_size, file) != byte_size) {
    perror("File Reading failed");
    fclose(file);
    free(buffer);
    return 1;
  }

  fclose(file);

  header Header;
  Header.a_text = le_32(buffer, 8);
  Header.a_data = le_32(buffer, 12);

  int text_start = 32;
  int data_start = 32 + Header.a_text;

  cpu_state_t cpu;
  init_cpu(&cpu);

  // TODO:
  // Set the Stack Segment

  // TODO:
  // fix this

  // Grab Text block, store it in memory in CS
  uint16_t text_offset = 0x0000;
  uint16_t cs_addr = cpu.registers[CS];
  for (int offset = text_start; offset < Header.a_text + text_start; offset++) {
    uint32_t addr = (cs_addr << 4) + text_offset;
    cpu.memory->data[addr] = buffer[offset];
    text_offset++;
  }

  // Grab Data block, store it in memory in DS
  uint16_t data_offset = 0x0000;
  uint16_t ds_addr = cpu.registers[DS];
  for (int offset = data_start; offset < Header.a_data + data_start; offset++) {
    uint32_t addr = (ds_addr << 4) + data_offset;
    cpu.memory->data[addr] = buffer[offset];
    data_offset++;
  }

  //
  // Go through the text block one by one
  // for (int offset = text_start; offset < Header.a_text + text_start;) {
  //   // Dissassembler need to fix this messy code
  //   cout << setfill(' ') << setw(6) << left
  //        << print_hex((uint16_t)(offset - text_start), 4) + ":";
  //
  //   instruction_info result_info = analyze_opcode(buffer, &offset);
  //
  //   print_result(result_info);
  //
  //   exec_parsed(&cpu, result_info);
  //   cout << "BX: " << hex << cpu.registers[3] << "\n";
  //
  //   offset += result_info.length;
  // }

  free(buffer);

  return 0;
}
