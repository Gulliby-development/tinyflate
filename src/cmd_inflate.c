#include <string.h>

#include "tinyflate.h"

static int report(tf_buf *err, const char *message) {
    tf_buf_puts(err, "tinyflate: ");
    tf_buf_puts(err, message);
    tf_buf_puts(err, "\n");
    return 1;
}

static int take_byte(const unsigned char *in, size_t len, size_t *pos, unsigned char *value) {
    if (*pos >= len)
        return 0;
    *value = in[(*pos)++];
    return 1;
}

static int take_u16le(const unsigned char *in, size_t len, size_t *pos, unsigned *value) {
    unsigned char a, b;
    if (!take_byte(in, len, pos, &a) || !take_byte(in, len, pos, &b))
        return 0;
    *value = (unsigned)a | ((unsigned)b << 8);
    return 1;
}

static int inflate_payload(const unsigned char *in, size_t len, size_t *pos, tf_buf *decoded, tf_buf *err) {
    size_t used = 0;
    const char *error = NULL;
    if (tf_inflate_raw(in + *pos, len - *pos, decoded, &used, &error) != 0)
        return report(err, error);
    *pos += used;
    return 0;
}

int tf_cmd_inflate(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err) {
    tf_buf decoded = {NULL, 0, 0};
    size_t pos = 0;
    const char *error = "unexpected end of input";
    unsigned char byte;
    if (opts->format == TF_FORMAT_GZIP) {
        unsigned char id1, id2, cm, flags;
        if (!take_byte(in, in_len, &pos, &id1))
            goto failed;
        if (id1 != 0x1f) {
            error = "not a gzip stream";
            goto failed;
        }
        if (!take_byte(in, in_len, &pos, &id2))
            goto failed;
        if (id2 != 0x8b) {
            error = "not a gzip stream";
            goto failed;
        }
        if (!take_byte(in, in_len, &pos, &cm))
            goto failed;
        if (cm != 8) {
            error = "unsupported compression method";
            goto failed;
        }
        if (!take_byte(in, in_len, &pos, &flags))
            goto failed;
        if ((flags & 0xe0u) != 0) {
            error = "reserved flag bits set";
            goto failed;
        }
        for (unsigned i = 0; i < 6; ++i)
            if (!take_byte(in, in_len, &pos, &byte))
                goto failed;
        if (flags & 4u) {
            unsigned xlen;
            if (!take_u16le(in, in_len, &pos, &xlen))
                goto failed;
            if ((size_t)xlen > in_len - pos)
                goto failed;
            pos += xlen;
        }
        for (unsigned flag = 8; flag <= 16; flag *= 2) {
            if (flags & flag) {
                do {
                    if (!take_byte(in, in_len, &pos, &byte))
                        goto failed;
                } while (byte != 0);
            }
        }
        if (flags & 2u) {
            unsigned crc;
            if (!take_u16le(in, in_len, &pos, &crc))
                goto failed;
            if ((tf_crc32(0, in, pos - 2) & 0xffffu) != crc) {
                error = "header checksum mismatch";
                goto failed;
            }
        }
        if (inflate_payload(in, in_len, &pos, &decoded, err))
            goto done;
        uint32_t crc = 0;
        if (in_len - pos < 4)
            goto failed;
        uint32_t expected_crc = (uint32_t)in[pos] | ((uint32_t)in[pos + 1] << 8) | ((uint32_t)in[pos + 2] << 16) |
                                ((uint32_t)in[pos + 3] << 24);
        pos += 4;
        crc = tf_crc32(crc, decoded.data, decoded.len);
        if (crc != expected_crc) {
            error = "crc mismatch";
            goto failed;
        }
        if (in_len - pos < 4)
            goto failed;
        uint32_t expected_size = (uint32_t)in[pos] | ((uint32_t)in[pos + 1] << 8) | ((uint32_t)in[pos + 2] << 16) |
                                 ((uint32_t)in[pos + 3] << 24);
        pos += 4;
        if ((uint32_t)decoded.len != expected_size) {
            error = "length mismatch";
            goto failed;
        }
    } else if (opts->format == TF_FORMAT_ZLIB) {
        unsigned char cmf, flg;
        if (!take_byte(in, in_len, &pos, &cmf))
            goto failed;
        if ((cmf & 15u) != 8) {
            error = "unsupported compression method";
            goto failed;
        }
        if ((cmf >> 4) > 7) {
            error = "unsupported window size";
            goto failed;
        }
        if (!take_byte(in, in_len, &pos, &flg))
            goto failed;
        if ((((unsigned)cmf << 8) | flg) % 31u != 0) {
            error = "bad zlib header check";
            goto failed;
        }
        if (flg & 0x20u) {
            error = "preset dictionary not supported";
            goto failed;
        }
        if (inflate_payload(in, in_len, &pos, &decoded, err))
            goto done;
        if (in_len - pos < 4)
            goto failed;
        uint32_t expected = ((uint32_t)in[pos] << 24) | ((uint32_t)in[pos + 1] << 16) | ((uint32_t)in[pos + 2] << 8) |
                            (uint32_t)in[pos + 3];
        pos += 4;
        if (tf_adler32(1, decoded.data, decoded.len) != expected) {
            error = "adler32 mismatch";
            goto failed;
        }
    } else {
        if (inflate_payload(in, in_len, &pos, &decoded, err))
            goto done;
    }
    if (pos != in_len) {
        error = "trailing data";
        goto failed;
    }
    tf_emit(opts, out, decoded.data, decoded.len);
    tf_buf_free(&decoded);
    return 0;
failed:
    report(err, error);
done:
    tf_buf_free(&decoded);
    return 1;
}
