
// @description:                           //

#ifndef SPARROW_UNICODEUTF8_H
#define SPARROW_UNICODEUTF8_H

/* ~ INCLUDE ~ */
#include <stdint.h>

/* ~ Functions ~ */
uint32_t GetByteNumOfEncodeUtf8(int value);
uint32_t GetByteNumOfDecodeUtf8(uint8_t byte);
uint8_t EncodeUtf8(uint8_t *buf, int value);
int DecodeUtf8(const uint8_t *bytePtr, uint32_t length);

#endif //SPARROW_UNICODEUTF8_H
