#include "tinyflate.h"

uint32_t tf_crc32(uint32_t crc, const unsigned char *data, size_t len) {
    crc ^= UINT32_C(0xffffffff);
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1) ^ (UINT32_C(0xedb88320) & mask);
        }
    }
    return crc ^ UINT32_C(0xffffffff);
}

uint32_t tf_adler32(uint32_t adler, const unsigned char *data, size_t len) {
    const uint32_t modulus = UINT32_C(65521);
    uint32_t a = adler & UINT32_C(0xffff);
    uint32_t b = adler >> 16;

    for (size_t i = 0; i < len; ++i) {
        a += data[i];
        if (a >= modulus) {
            a -= modulus;
        }
        b += a;
        if (b >= modulus) {
            b -= modulus;
        }
    }
    return (b << 16) | a;
}
