#ifndef TINYFLATE_H
#define TINYFLATE_H

#include <stddef.h>
#include <stdint.h>

#define TF_VERSION "0.1.0"

#define TF_FORMAT_RAW 0
#define TF_FORMAT_GZIP 1
#define TF_FORMAT_ZLIB 2

#define TF_LEVEL_STORED 0
#define TF_LEVEL_FIXED 1

#define TF_OPT_FORMAT 1u
#define TF_OPT_LEVEL 2u

/* A growing byte buffer; data is NULL while len is 0. */
typedef struct {
    unsigned char *data;
    size_t len;
    size_t cap;
} tf_buf;

/* What one command line produced: the exit status and the bytes for standard output and standard error. */
typedef struct {
    int status;
    unsigned char *out;
    size_t out_len;
    unsigned char *err;
    size_t err_len;
} tf_result;

/* The options of a command line after the command word. */
typedef struct {
    int hex;
    int format;
    int level;
} tf_options;

/* A command writes its output to out and returns 0, or writes "tinyflate: TEXT\n" to err and returns 1. */
typedef int (*tf_command_fn)(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err);

typedef struct {
    const char *name;
    tf_command_fn run;
    unsigned options;
} tf_command;

extern const tf_command tf_commands[];

/* Runs a command line (the words after the program name) on the given standard input. Free the result with tf_result_free. */
tf_result tf_run(int argc, const char *const *argv, const unsigned char *in, size_t in_len);
void tf_result_free(tf_result *result);

/* Whether the command line is a valid use of a command that reads standard input. */
int tf_reads_stdin(int argc, const char *const *argv);

void tf_buf_put(tf_buf *buf, const void *data, size_t len);
void tf_buf_puts(tf_buf *buf, const char *text);
void tf_buf_free(tf_buf *buf);

/* Writes data to out: the bytes themselves, or lowercase hex digits and a newline when opts->hex is set. */
void tf_emit(const tf_options *opts, tf_buf *out, const unsigned char *data, size_t len);

uint32_t tf_crc32(uint32_t crc, const unsigned char *data, size_t len);
uint32_t tf_adler32(uint32_t adler, const unsigned char *data, size_t len);

/* Decodes a raw DEFLATE stream that starts at data[0]; on success returns 0, appends the bytes to out and sets *used to
   the number of input bytes the stream took. On failure returns 1 and sets *error to a static message. */
int tf_inflate_raw(const unsigned char *data, size_t len, tf_buf *out, size_t *used, const char **error);

/* Encodes data as one raw DEFLATE stream (TF_LEVEL_STORED or TF_LEVEL_FIXED) appended to out. */
void tf_deflate_raw(const unsigned char *data, size_t len, int level, tf_buf *out);

int tf_cmd_crc32(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err);
int tf_cmd_adler32(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err);
int tf_cmd_inflate(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err);
int tf_cmd_deflate(const tf_options *opts, const unsigned char *in, size_t in_len, tf_buf *out, tf_buf *err);

#endif
