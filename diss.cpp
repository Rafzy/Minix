#include "opcode.hpp"
#include "utils.hpp"
#include <cstdint>
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

  if (argc != 2) {
    printf("Argument format wrong");
    return 1;
  }

  file = fopen(argv[1], "rb");
  if (!file) {
    perror("Reading File Failed");
    return 1;
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

  int offset = 0;
  for (int i = 32; i < Header.a_text + 32; i++) {
    analyze_opcode(buffer, i);
  }
  // analyze_opcode(buffer, 32);

  free(buffer);

  return 0;
}
