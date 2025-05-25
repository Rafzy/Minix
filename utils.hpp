#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>

// Handle Little Endianness
uint32_t le_32(uint8_t *bytes, int offset);
uint16_t le_16(uint8_t *bytes, int offset);

char *get_mnemonic();

#endif // !UTILS_H
