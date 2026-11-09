#include "tinyflate.h"

static void append_u32_le(tf_buf *buf, uint32_t value) {
    unsigned char bytes[4];
    bytes[0] = (unsigned char)value;
    bytes[1] = (unsigned char)(value >> 8);
    bytes[2] = (unsigned char)(value >> 16);
    bytes[3] = (unsigned char)(value >> 24);
    tf_buf_put(buf, bytes, sizeof(bytes));
}

static void append_u32_be(tf_buf *buf, uint32_t value) {
    unsigned char bytes[4];
    bytes[0] = (unsigned char)(value >> 24);
    bytes[1] = (unsigned char)(value >> 16);
    bytes[2] = (unsigned char)(value >> 8);
    bytes[3] = (unsigned char)value;
    tf_buf_put(buf, bytes, sizeof(bytes));
}

int tf_cmd_deflate(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err) {
    static const unsigned char gzip_header[] = {0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};
    static const unsigned char zlib_header[] = {0x78, 0x01};
    tf_buf stream = {NULL, 0, 0};
    tf_buf wrapped = {NULL, 0, 0};
    (void)err;

    tf_deflate_raw(in, in_len, opts->level, &stream);
    if (opts->format == TF_FORMAT_GZIP) {
        tf_buf_put(&wrapped, gzip_header, sizeof(gzip_header));
        tf_buf_put(&wrapped, stream.data, stream.len);
        append_u32_le(&wrapped, tf_crc32(0, in, in_len));
        append_u32_le(&wrapped, (uint32_t)in_len);
    } else if (opts->format == TF_FORMAT_ZLIB) {
        tf_buf_put(&wrapped, zlib_header, sizeof(zlib_header));
        tf_buf_put(&wrapped, stream.data, stream.len);
        append_u32_be(&wrapped, tf_adler32(1, in, in_len));
    } else {
        tf_buf_put(&wrapped, stream.data, stream.len);
    }

    tf_emit(opts, out, wrapped.data, wrapped.len);
    tf_buf_free(&wrapped);
    tf_buf_free(&stream);
    return 0;
}
