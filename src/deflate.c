#include <stddef.h>
#include <stdint.h>

#include "tinyflate.h"

typedef struct {
    tf_buf *out;
    unsigned char byte;
    unsigned used;
} bit_writer;

static void write_bit(bit_writer *writer, unsigned bit) {
    writer->byte = (unsigned char)(writer->byte | ((bit & 1u) << writer->used));
    ++writer->used;
    if (writer->used == 8) {
        tf_buf_put(writer->out, &writer->byte, 1);
        writer->byte = 0;
        writer->used = 0;
    }
}

static void write_bits(bit_writer *writer, unsigned value, unsigned count) {
    for (unsigned i = 0; i < count; ++i) {
        write_bit(writer, value >> i);
    }
}

static void write_code(bit_writer *writer, unsigned code, unsigned count) {
    for (unsigned i = count; i > 0; --i) {
        write_bit(writer, code >> (i - 1));
    }
}

static void finish_bits(bit_writer *writer) {
    if (writer->used != 0) {
        tf_buf_put(writer->out, &writer->byte, 1);
        writer->byte = 0;
        writer->used = 0;
    }
}

static void write_fixed_symbol(bit_writer *writer, unsigned symbol) {
    unsigned code;
    unsigned count;
    if (symbol <= 143) {
        code = 0x30u + symbol;
        count = 8;
    } else if (symbol <= 255) {
        code = 0x190u + symbol - 144u;
        count = 9;
    } else if (symbol <= 279) {
        code = symbol - 256u;
        count = 7;
    } else {
        code = 0xc0u + symbol - 280u;
        count = 8;
    }
    write_code(writer, code, count);
}

static void write_length_distance(bit_writer *writer, size_t length, size_t distance) {
    static const unsigned short length_base[29] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const unsigned char length_extra[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const unsigned short distance_base[30] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
        1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
    static const unsigned char distance_extra[30] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    unsigned length_code = 0;
    while (length_code < 28 && length >= length_base[length_code + 1]) {
        ++length_code;
    }
    write_fixed_symbol(writer, 257u + length_code);
    write_bits(writer, (unsigned)(length - length_base[length_code]), length_extra[length_code]);

    unsigned distance_code = 0;
    while (distance_code < 29 && distance >= distance_base[distance_code + 1]) {
        ++distance_code;
    }
    write_code(writer, distance_code, 5);
    write_bits(writer, (unsigned)(distance - distance_base[distance_code]), distance_extra[distance_code]);
}

static void write_fixed_data(const unsigned char *data, size_t len, tf_buf *out) {
    bit_writer writer = {out, 0, 0};
    write_bits(&writer, 3, 3);
    for (size_t i = 0; i < len;) {
        size_t max_length = len - i < 258 ? len - i : 258;
        size_t max_distance = i < 32768 ? i : 32768;
        size_t best_length = 0;
        size_t best_distance = 0;
        for (size_t distance = 1; distance <= max_distance; ++distance) {
            size_t match = 0;
            while (match < max_length && data[i - distance + match] == data[i + match]) {
                ++match;
            }
            if (match > best_length) {
                best_length = match;
                best_distance = distance;
            }
        }
        if (best_length >= 3) {
            write_length_distance(&writer, best_length, best_distance);
            i += best_length;
        } else {
            write_fixed_symbol(&writer, data[i]);
            ++i;
        }
    }
    write_fixed_symbol(&writer, 256);
    finish_bits(&writer);
}

void tf_deflate_raw(const unsigned char *data, size_t len, int level, tf_buf *out) {
    if (level == TF_LEVEL_STORED) {
        size_t offset = 0;
        if (len == 0) {
            static const unsigned char empty_block[] = {1, 0, 0, 0xff, 0xff};
            tf_buf_put(out, empty_block, sizeof(empty_block));
            return;
        }
        while (offset < len) {
            size_t remaining = len - offset;
            size_t block_len = remaining > 65535 ? 65535 : remaining;
            unsigned char header[5];
            uint16_t size = (uint16_t)block_len;
            uint16_t complement = (uint16_t)~size;
            header[0] = offset + block_len == len ? 1 : 0;
            header[1] = (unsigned char)size;
            header[2] = (unsigned char)(size >> 8);
            header[3] = (unsigned char)complement;
            header[4] = (unsigned char)(complement >> 8);
            tf_buf_put(out, header, sizeof(header));
            tf_buf_put(out, data + offset, block_len);
            offset += block_len;
        }
        return;
    }
    write_fixed_data(data, len, out);
}
