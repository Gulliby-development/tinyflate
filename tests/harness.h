#ifndef TF_HARNESS_H
#define TF_HARNESS_H

#include <stdio.h>
#include <string.h>

#include "tinyflate.h"

/* ARGS("a", "b") is the pair argv, argc of a command line; NOARGS is the empty command line. */
#define ARGS(...) (const char *const[]){__VA_ARGS__}, (int)(sizeof((const char *const[]){__VA_ARGS__}) / sizeof(const char *))
#define NOARGS NULL, 0

/* row_at runs the command line with the text as standard input and compares the exit status, the standard output and
   the standard error byte for byte; it returns 0 when they are equal and 1 otherwise. */
static inline int row_at(const char *file, int line, const char *const *argv, int argc, const char *input, int status,
                         const char *out, const char *err) {
    tf_result result = tf_run(argc, argv, (const unsigned char *)input, strlen(input));
    int ok = result.status == status && result.out_len == strlen(out) && result.err_len == strlen(err) &&
             (result.out_len == 0 || memcmp(result.out, out, result.out_len) == 0) &&
             (result.err_len == 0 || memcmp(result.err, err, result.err_len) == 0);
    if (!ok) {
        fprintf(stderr, "%s:%d: row failed: status %d, stdout \"%.*s\", stderr \"%.*s\"\n", file, line, result.status,
                (int)result.out_len, result.out == NULL ? "" : (const char *)result.out, (int)result.err_len,
                result.err == NULL ? "" : (const char *)result.err);
    }
    tf_result_free(&result);
    return ok ? 0 : 1;
}

/* reads_at compares tf_reads_stdin with the expected value (1 or 0). */
static inline int reads_at(const char *file, int line, const char *const *argv, int argc, int expected) {
    int got = tf_reads_stdin(argc, argv) != 0;
    if (got != expected) {
        fprintf(stderr, "%s:%d: reads failed: got %d, expected %d\n", file, line, got, expected);
    }
    return got == expected ? 0 : 1;
}

#define row(...) row_at(__FILE__, __LINE__, __VA_ARGS__)
#define reads(...) reads_at(__FILE__, __LINE__, __VA_ARGS__)

#endif
