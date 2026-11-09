#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyflate.h"

static void allocation_failure(void) {
    fputs("tinyflate: out of memory\n", stderr);
    exit(EXIT_FAILURE);
}

void tf_buf_put(tf_buf *buf, const void *data, size_t len) {
    if (len == 0) {
        return;
    }
    if (buf == NULL || data == NULL || len > SIZE_MAX - buf->len) {
        allocation_failure();
    }

    size_t needed = buf->len + len;
    if (needed > buf->cap) {
        size_t capacity = buf->cap == 0 ? 64 : buf->cap;
        while (capacity < needed) {
            if (capacity > SIZE_MAX / 2) {
                capacity = needed;
                break;
            }
            capacity *= 2;
        }
        unsigned char *grown = realloc(buf->data, capacity);
        if (grown == NULL) {
            allocation_failure();
        }
        buf->data = grown;
        buf->cap = capacity;
    }

    memcpy(buf->data + buf->len, data, len);
    buf->len = needed;
}

void tf_buf_puts(tf_buf *buf, const char *text) {
    tf_buf_put(buf, text, strlen(text));
}

void tf_buf_free(tf_buf *buf) {
    if (buf == NULL) {
        return;
    }
    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

void tf_emit(const tf_options *opts, tf_buf *out, const unsigned char *data, size_t len) {
    static const unsigned char digits[] = "0123456789abcdef";

    if (opts == NULL || !opts->hex) {
        tf_buf_put(out, data, len);
        return;
    }

    for (size_t i = 0; i < len; ++i) {
        unsigned char pair[2];
        pair[0] = digits[data[i] >> 4];
        pair[1] = digits[data[i] & 0x0f];
        tf_buf_put(out, pair, sizeof(pair));
    }
    tf_buf_put(out, "\n", 1);
}
