#include <string.h>

#include "tinyflate.h"

#define MAX_BITS 15

typedef struct {
    const unsigned char *data;
    size_t len;
    size_t pos;
    unsigned bit;
} bit_reader;

typedef struct {
    unsigned count[MAX_BITS + 1];
    unsigned first[MAX_BITS + 1];
    unsigned offset[MAX_BITS + 1];
    unsigned symbols[288];
    unsigned total;
    unsigned max_bits;
} huffman;

static const char *const unexpected = "unexpected end of input";
static const char *const bad_table = "bad huffman table";
static const char *const bad_symbol = "bad symbol";
static const char *const bad_distance = "bad distance";

static int get_bits(bit_reader *r, unsigned n, unsigned *value) {
    unsigned result = 0;
    for (unsigned i = 0; i < n; ++i) {
        if (r->pos >= r->len) {
            return 0;
        }
        result |= ((unsigned)((r->data[r->pos] >> r->bit) & 1u)) << i;
        if (++r->bit == 8) {
            r->bit = 0;
            ++r->pos;
        }
    }
    *value = result;
    return 1;
}

static int build_huffman(huffman *h, const unsigned char *lengths, unsigned n, int allow_incomplete) {
    unsigned offsets[MAX_BITS + 1] = {0};
    unsigned left = 1;
    memset(h, 0, sizeof(*h));
    for (unsigned i = 0; i < n; ++i) {
        if (lengths[i] > MAX_BITS) {
            return 0;
        }
        ++h->count[lengths[i]];
        if (lengths[i] > h->max_bits) {
            h->max_bits = lengths[i];
        }
    }
    h->count[0] = 0;
    for (unsigned bits = 1; bits <= MAX_BITS; ++bits) {
        unsigned available = left << 1;
        if (h->count[bits] > available) {
            return 0;
        }
        left = available - h->count[bits];
    }
    h->total = n;
    for (unsigned bits = 1; bits <= MAX_BITS; ++bits) {
        h->first[bits] = (h->first[bits - 1] + h->count[bits - 1]) << 1;
        offsets[bits] = offsets[bits - 1] + h->count[bits - 1];
        h->offset[bits] = offsets[bits];
    }
    unsigned next[MAX_BITS + 1];
    memcpy(next, offsets, sizeof(next));
    for (unsigned symbol = 0; symbol < n; ++symbol) {
        unsigned length = lengths[symbol];
        if (length != 0) {
            h->symbols[next[length]++] = symbol;
        }
    }
    if (left != 0) {
        unsigned used = 0;
        unsigned only_length = 0;
        for (unsigned bits = 1; bits <= MAX_BITS; ++bits) {
            used += h->count[bits];
            if (h->count[bits] != 0) {
                only_length = bits;
            }
        }
        if (!(allow_incomplete && (used == 0 || (used == 1 && only_length == 1)))) {
            return 0;
        }
    }
    return 1;
}

static int decode_symbol(bit_reader *r, const huffman *h, unsigned *symbol) {
    unsigned code = 0;
    for (unsigned bits = 1; bits <= h->max_bits; ++bits) {
        unsigned bit;
        if (!get_bits(r, 1, &bit)) {
            return -1;
        }
        code = (code << 1) | bit;
        unsigned first = h->first[bits];
        if (code >= first && code - first < h->count[bits]) {
            *symbol = h->symbols[h->offset[bits] + code - first];
            return 1;
        }
    }
    return 0;
}

static int fail(const char **error, const char *message) {
    *error = message;
    return 1;
}

static int compressed(bit_reader *r, const huffman *lit, const huffman *dist, tf_buf *out, const char **error) {
    static const unsigned length_base[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27,
                                             31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const unsigned length_extra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3,
                                              3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const unsigned dist_base[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129,
                                           193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
                                           8193, 12289, 16385, 24577};
    static const unsigned dist_extra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                            7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
    for (;;) {
        unsigned sym;
        int decoded = decode_symbol(r, lit, &sym);
        if (decoded < 0)
            return fail(error, unexpected);
        if (decoded == 0)
            return fail(error, bad_symbol);
        if (sym < 256) {
            unsigned char byte = (unsigned char)sym;
            tf_buf_put(out, &byte, 1);
            continue;
        }
        if (sym == 256)
            return 0;
        if (sym < 257 || sym > 285)
            return fail(error, bad_symbol);
        unsigned ix = sym - 257, extra = 0, length = length_base[ix];
        if (!get_bits(r, length_extra[ix], &extra))
            return fail(error, unexpected);
        length += extra;
        decoded = decode_symbol(r, dist, &sym);
        if (decoded < 0)
            return fail(error, unexpected);
        if (decoded == 0 || sym >= 30)
            return fail(error, bad_symbol);
        extra = 0;
        if (!get_bits(r, dist_extra[sym], &extra))
            return fail(error, unexpected);
        size_t distance = (size_t)dist_base[sym] + extra;
        if (distance > out->len)
            return fail(error, bad_distance);
        for (size_t i = 0; i < length; ++i) {
            unsigned char byte = out->data[out->len - distance];
            tf_buf_put(out, &byte, 1);
        }
    }
}

int tf_inflate_raw(const unsigned char *data, size_t len, tf_buf *out, size_t *used, const char **error) {
    bit_reader r = {data, len, 0, 0};
    tf_buf result = {NULL, 0, 0};
    int final = 0;
    *used = 0;
    *error = unexpected;
    while (!final) {
        unsigned value, type;
        if (!get_bits(&r, 1, &value))
            goto truncated;
        final = (int)value;
        if (!get_bits(&r, 2, &type))
            goto truncated;
        if (type == 3) {
            *error = "invalid block type";
            goto failed;
        }
        if (type == 0) {
            if (r.bit != 0) {
                r.bit = 0;
                ++r.pos;
            }
            if (r.pos >= r.len)
                goto truncated;
            unsigned length, inverse;
            if (!get_bits(&r, 16, &length) || !get_bits(&r, 16, &inverse))
                goto truncated;
            if ((length ^ inverse) != 0xffffu) {
                *error = "bad stored length";
                goto failed;
            }
            if ((size_t)length > r.len - r.pos)
                goto truncated;
            tf_buf_put(&result, r.data + r.pos, length);
            r.pos += length;
            continue;
        }
        huffman lit, dist;
        unsigned char lengths[320] = {0};
        if (type == 1) {
            for (unsigned i = 0; i <= 143; ++i)
                lengths[i] = 8;
            for (unsigned i = 144; i <= 255; ++i)
                lengths[i] = 9;
            for (unsigned i = 256; i <= 279; ++i)
                lengths[i] = 7;
            for (unsigned i = 280; i <= 287; ++i)
                lengths[i] = 8;
            if (!build_huffman(&lit, lengths, 288, 0)) {
                *error = bad_table;
                goto failed;
            }
            for (unsigned i = 0; i < 32; ++i)
                lengths[i] = 5;
            if (!build_huffman(&dist, lengths, 32, 0)) {
                *error = bad_table;
                goto failed;
            }
        } else {
            unsigned hlit, hdist, hclen;
            if (!get_bits(&r, 5, &hlit) || !get_bits(&r, 5, &hdist) || !get_bits(&r, 4, &hclen))
                goto truncated;
            hlit += 257;
            hdist += 1;
            hclen += 4;
            if (hlit > 286 || hdist > 30) {
                *error = bad_table;
                goto failed;
            }
            static const unsigned order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            unsigned char cl[19] = {0};
            for (unsigned i = 0; i < hclen; ++i) {
                if (!get_bits(&r, 3, &value))
                    goto truncated;
                cl[order[i]] = (unsigned char)value;
            }
            huffman ch;
            if (!build_huffman(&ch, cl, 19, 0)) {
                *error = bad_table;
                goto failed;
            }
            unsigned total = hlit + hdist, at = 0;
            while (at < total) {
                unsigned sym;
                int d = decode_symbol(&r, &ch, &sym);
                if (d < 0)
                    goto truncated;
                if (d == 0) {
                    *error = bad_symbol;
                    goto failed;
                }
                if (sym <= 15)
                    lengths[at++] = (unsigned char)sym;
                else {
                    unsigned repeat, count;
                    if (sym == 16) {
                        if (at == 0) {
                            *error = bad_table;
                            goto failed;
                        }
                        if (!get_bits(&r, 2, &repeat))
                            goto truncated;
                        count = repeat + 3;
                        value = lengths[at - 1];
                    } else if (sym == 17) {
                        if (!get_bits(&r, 3, &repeat))
                            goto truncated;
                        count = repeat + 3;
                        value = 0;
                    } else if (sym == 18) {
                        if (!get_bits(&r, 7, &repeat))
                            goto truncated;
                        count = repeat + 11;
                        value = 0;
                    } else {
                        *error = bad_symbol;
                        goto failed;
                    }
                    if (count > total - at) {
                        *error = bad_table;
                        goto failed;
                    }
                    while (count-- != 0)
                        lengths[at++] = (unsigned char)value;
                }
            }
            if (lengths[256] == 0) {
                *error = bad_table;
                goto failed;
            }
            if (!build_huffman(&lit, lengths, hlit, 1)) {
                *error = bad_table;
                goto failed;
            }
            if (!build_huffman(&dist, lengths + hlit, hdist, 1)) {
                *error = bad_table;
                goto failed;
            }
        }
        if (compressed(&r, &lit, &dist, &result, error) != 0)
            goto failed;
    }
    *used = r.pos + (r.bit != 0 ? 1u : 0u);
    tf_buf_put(out, result.data, result.len);
    tf_buf_free(&result);
    return 0;
truncated:
    *error = unexpected;
failed:
    tf_buf_free(&result);
    return 1;
}
