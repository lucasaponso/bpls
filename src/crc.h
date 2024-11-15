#ifndef CRC_H
#define CRC_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static void crc16(uint8_t ch, uint16_t *crc);
uint16_t slipcrc(uint8_t *buf, int nbuf);
int validate_crc(uint8_t * output_buffer, int len);


#endif //CRC_H