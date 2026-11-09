#include <stdio.h>
#include <stdlib.h>

#include "tinyflate.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

static unsigned char *read_stdin(size_t *len) {
    size_t cap = 4096;
    unsigned char *data = malloc(cap);
    *len = 0;
    if (data == NULL) {
        return NULL;
    }
    for (;;) {
        size_t got = fread(data + *len, 1, cap - *len, stdin);
        *len += got;
        if (*len < cap) {
            break;
        }
        cap *= 2;
        unsigned char *bigger = realloc(data, cap);
        if (bigger == NULL) {
            free(data);
            return NULL;
        }
        data = bigger;
    }
    return data;
}

int main(int argc, char **argv) {
    const char *const *words = (const char *const *)(argv + 1);
    unsigned char *input = NULL;
    size_t input_len = 0;
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
    if (tf_reads_stdin(argc - 1, words)) {
        input = read_stdin(&input_len);
        if (input == NULL) {
            fputs("tinyflate: out of memory\n", stderr);
            return 1;
        }
    }
    tf_result result = tf_run(argc - 1, words, input, input_len);
    if (result.out_len > 0) {
        fwrite(result.out, 1, result.out_len, stdout);
    }
    if (result.err_len > 0) {
        fwrite(result.err, 1, result.err_len, stderr);
    }
    fflush(stdout);
    int status = result.status;
    tf_result_free(&result);
    free(input);
    return status;
}
