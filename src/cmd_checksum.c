#include "tinyflate.h"

static void append_checksum(tf_buf *out, uint32_t value) {
    static const unsigned char digits[] = "0123456789abcdef";
    unsigned char text[9];

    for (unsigned i = 0; i < 8; ++i) {
        unsigned shift = 28u - i * 4u;
        text[i] = digits[(value >> shift) & 0x0fu];
    }
    text[8] = '\n';
    tf_buf_put(out, text, sizeof(text));
}

int tf_cmd_crc32(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err) {
    (void)opts;
    (void)err;
    append_checksum(out, tf_crc32(0, in, in_len));
    return 0;
}

int tf_cmd_adler32(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err) {
    (void)opts;
    (void)err;
    append_checksum(out, tf_adler32(1, in, in_len));
    return 0;
}
