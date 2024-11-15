#include "crc.h"
/*
int validate_crc(uint8_t * output_buffer, int len)

Notes: 
Gets last two bytes of buffer (CRC)
and compares with the calculated CRC (using slipcrc).
If both sets of bytes match then the CRC is valid.
*/
int validate_crc(uint8_t * output_buffer, int len)
{
    uint16_t * p;
    p = (uint16_t *)&output_buffer[len - 2];
    uint16_t crc = slipcrc(output_buffer, len - 2);

    printf("CRC %04x ", crc);
    
    if (crc == *p)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/*
 * slipcrc - Calculate the CRC-16-CCITT checksum for a given buffer of data.
 *
 * This function computes the CRC-16-CCITT checksum over the provided data buffer.
 * The CRC is calculated using the `crc16` function for each byte in the buffer.
 * The final checksum is returned in the form of a 16-bit value, with the result 
 * being bitwise inverted before being returned.
 *
 * Parameters:
 *   uint8_t *buf    - Pointer to the input buffer containing the data to be checksummed.
 *   int nbuf        - Number of bytes in the input buffer (`buf`).
 *
 * Returns:
 *   uint16_t        - The computed 16-bit CRC checksum, inverted.
 *
 * Notes:
 *   - The CRC is calculated in a byte-by-byte fashion, starting from the first byte of the buffer.
 *   - The CRC-16-CCITT algorithm uses the polynomial 0x11021 and is commonly used in communication protocols.
 *   - The result is the 16-bit checksum in little-endian format (lower byte first).
 *   - The checksum is bitwise negated before being returned.
 *   - The function does not modify the input buffer or rely on external state, other than a static `crcbuf` variable.
 */
uint16_t slipcrc(uint8_t *buf, int nbuf)
{
    static uint8_t crcbuf[2];
    uint16_t crc = 0;
    while( --nbuf >= 0 )
        crc16(*buf++, &crc);
    crc = ~crc;
    crcbuf[0] = (crc) & 255;
    crcbuf[1] = (crc >> 8) & 255;
    return crc;
}

/*
 * crc16 - Calculate CRC-16-CCITT checksum for a single byte of data.
 *
 * This function computes the CRC-16-CCITT checksum for one byte of data. The
 * CRC is calculated by shifting the input byte and the current CRC value bit by bit.
 * If a bit mismatch is detected between the input byte and the current CRC value, 
 * the CRC value is updated with an XOR operation using the polynomial 0x8408, 
 * which is the bit-reversed form of the standard 0x11021 polynomial used in CRC-16-CCITT.
 *
 * Parameters:
 *   uint8_t ch   - The byte of data for which the CRC is calculated.
 *   uint16_t *crc - Pointer to the current CRC value, which is updated by the function.
 *
 * Returns:
 *   void         - The function does not return a value, but updates the CRC value
 *                 pointed to by `crc`.
 *
 * Notes:
 *   - The function processes the byte `ch` bit by bit, and updates the CRC value
 *     (pointed to by `crc`) after each bit.
 *   - The polynomial used for the CRC calculation is 0x8408, which is the bit-reversed
 *     form of the CRC-16-CCITT standard polynomial 0x11021.
 *   - This function modifies the CRC value in-place, which means the result is stored
 *     directly in the variable pointed to by `crc`.
 *   - The function assumes that the initial CRC value is set to 0 and the result is
 *     updated with each byte processed.
 */

static void crc16(uint8_t ch, uint16_t *crc)
{
    uint8_t bits = 8;
    uint16_t c = *crc;
    do
    {
        if( (ch & 1) ^ (c & 1) )
        {
            c >>= 1;
            c ^= 0x8408;
        }
        else
        {
            c >>= 1;
        }

        ch >>= 1;
    } while( --bits );
    *crc = c;
}
