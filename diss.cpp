#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

void extract_bytes(uint8_t byte) {

};

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

  uint32_t full_ptr = 0;
  for (int i = 11; i > 7; i--) {
    full_ptr = full_ptr << 8 | buffer[i];
  }

  int offset = 0;

  printf("%x", full_ptr);

  for (int i = 32; i < full_ptr + 32; i++) {
    switch (buffer[i]) {
    case 0xb8:
    case 0xb9:
    case 0xba:
    case 0xbb:
    case 0xbc:
    case 0xbd:
    case 0xbe:
    case 0xbf: {
      // printf("%04x: ", offset);
      // printf("%02x%02x%02x", buffer[i], buffer[i + 1], buffer[i + 2]);
      // printf("%15s, %02x%02x\n", "mov bx", buffer[i + 2], buffer[i + 1]);
      uint32_t test;
      test = buffer[i + 2] << 16 | buffer[i + 1] << 8 | buffer[i];
      uint32_t check_w = 0x100;
      uint32_t check_d = 0x200;
      if ((test & check_d) == 0x200) {
        printf("d = 1");
      }
      if ((test & check_w) == 0x100) {
        printf("w = 1");
      }

      i += 2;
      offset += 3;
      break;
    }
    case 0xcd: {
      printf("%04x: ", offset);
      printf("%02x%02x", buffer[i], buffer[i + 1]);
      printf("%14s %02x\n", "int", buffer[i + 1]);
      i += 1;
      offset += 2;
      break;
    }
    case 0x00: {
      printf("%04x: ", offset);
      printf("%02x%02x", buffer[i], buffer[i + 1]);
      printf("%28s\n", "add [bx + si], al");
      i += 1;
      offset += 2;
      break;
    }
    }
  }

  free(buffer);

  return 0;
}
