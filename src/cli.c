#include <string.h>

#include "tinyflate.h"

static const char usage[] =
    "usage: tinyflate crc32 [--hex]\n"
    "       tinyflate adler32 [--hex]\n"
    "       tinyflate inflate [--format raw|gzip|zlib] [--hex]\n"
    "       tinyflate deflate [--format raw|gzip|zlib] [--level stored|fixed] [--hex]\n";

static const char *word_or_empty(const char *word) {
    return word == NULL ? "" : word;
}

static const tf_command *find_command(const char *name) {
    for (const tf_command *command = tf_commands; command->name != NULL; ++command) {
        if (strcmp(command->name, name) == 0) {
            return command;
        }
    }
    return NULL;
}

static void report_word(tf_buf *err, const char *prefix, const char *word, const char *suffix) {
    if (err != NULL) {
        tf_buf_puts(err, prefix);
        tf_buf_puts(err, word_or_empty(word));
        tf_buf_puts(err, suffix);
    }
}

static int validate_options(const tf_command *command, int argc, const char *const *argv, tf_options *opts,
                            tf_buf *err) {
    int seen_hex = 0;
    int seen_format = 0;
    int seen_level = 0;
    opts->hex = 0;
    opts->format = TF_FORMAT_RAW;
    opts->level = TF_LEVEL_FIXED;

    for (int i = 1; i < argc; ++i) {
        const char *word = word_or_empty(argv[i]);
        if (word[0] != '-' || word[1] == '\0') {
            report_word(err, "tinyflate: unexpected argument ", word, "\n");
            return 0;
        }

        if (strcmp(word, "--hex") == 0) {
            if (seen_hex) {
                report_word(err, "tinyflate: ", word, " given twice\n");
                return 0;
            }
            seen_hex = 1;
            opts->hex = 1;
        } else if (strcmp(word, "--format") == 0 && (command->options & TF_OPT_FORMAT) != 0) {
            if (seen_format) {
                report_word(err, "tinyflate: ", word, " given twice\n");
                return 0;
            }
            seen_format = 1;
            if (i + 1 >= argc) {
                if (err != NULL) {
                    tf_buf_puts(err, "tinyflate: --format needs one of raw, gzip, zlib\n");
                }
                return 0;
            }
            const char *value = word_or_empty(argv[++i]);
            if (strcmp(value, "raw") == 0) {
                opts->format = TF_FORMAT_RAW;
            } else if (strcmp(value, "gzip") == 0) {
                opts->format = TF_FORMAT_GZIP;
            } else if (strcmp(value, "zlib") == 0) {
                opts->format = TF_FORMAT_ZLIB;
            } else {
                if (err != NULL) {
                    tf_buf_puts(err, "tinyflate: --format needs one of raw, gzip, zlib\n");
                }
                return 0;
            }
        } else if (strcmp(word, "--level") == 0 && (command->options & TF_OPT_LEVEL) != 0) {
            if (seen_level) {
                report_word(err, "tinyflate: ", word, " given twice\n");
                return 0;
            }
            seen_level = 1;
            if (i + 1 >= argc) {
                if (err != NULL) {
                    tf_buf_puts(err, "tinyflate: --level needs one of stored, fixed\n");
                }
                return 0;
            }
            const char *value = word_or_empty(argv[++i]);
            if (strcmp(value, "stored") == 0) {
                opts->level = TF_LEVEL_STORED;
            } else if (strcmp(value, "fixed") == 0) {
                opts->level = TF_LEVEL_FIXED;
            } else {
                if (err != NULL) {
                    tf_buf_puts(err, "tinyflate: --level needs one of stored, fixed\n");
                }
                return 0;
            }
        } else {
            report_word(err, "tinyflate: unknown option ", word, "\n");
            return 0;
        }
    }
    return 1;
}

static int validate_command(int argc, const char *const *argv, const tf_command **command, tf_options *opts,
                            tf_buf *err) {
    if (argc <= 0 || argv == NULL) {
        if (err != NULL) {
            tf_buf_puts(err, usage);
        }
        return 0;
    }

    const char *name = word_or_empty(argv[0]);
    *command = find_command(name);
    if (*command == NULL) {
        report_word(err, "tinyflate: unknown command ", name, "\n");
        return 0;
    }
    return validate_options(*command, argc, argv, opts, err);
}

static int hex_value(unsigned char byte) {
    if (byte >= '0' && byte <= '9') {
        return (int)(byte - '0');
    }
    if (byte >= 'a' && byte <= 'f') {
        return (int)(byte - 'a') + 10;
    }
    if (byte >= 'A' && byte <= 'F') {
        return (int)(byte - 'A') + 10;
    }
    return -1;
}

static int decode_hex(const unsigned char *input, size_t input_len, tf_buf *decoded) {
    int high = -1;
    for (size_t i = 0; i < input_len; ++i) {
        unsigned char byte = input[i];
        if (byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n') {
            continue;
        }
        int value = hex_value(byte);
        if (value < 0) {
            return 0;
        }
        if (high < 0) {
            high = value;
        } else {
            unsigned char decoded_byte = (unsigned char)((high << 4) | value);
            tf_buf_put(decoded, &decoded_byte, 1);
            high = -1;
        }
    }
    return high < 0;
}

tf_result tf_run(int argc, const char *const *argv, const unsigned char *in, size_t in_len) {
    tf_result result = {2, NULL, 0, NULL, 0};
    tf_buf out = {NULL, 0, 0};
    tf_buf err = {NULL, 0, 0};
    tf_buf decoded = {NULL, 0, 0};
    const tf_command *command = NULL;
    tf_options opts = {0, TF_FORMAT_RAW, TF_LEVEL_FIXED};

    if (argc <= 0 || argv == NULL) {
        tf_buf_puts(&err, usage);
    } else {
        const char *name = word_or_empty(argv[0]);
        if (strcmp(name, "help") == 0 || strcmp(name, "-h") == 0 || strcmp(name, "--help") == 0) {
            tf_buf_puts(&out, usage);
            result.status = 0;
        } else if (strcmp(name, "--version") == 0) {
            tf_buf_puts(&out, "tinyflate " TF_VERSION "\n");
            result.status = 0;
        } else if (!validate_command(argc, argv, &command, &opts, &err)) {
            result.status = 2;
        } else if (opts.hex && !decode_hex(in, in_len, &decoded)) {
            tf_buf_puts(&err, "tinyflate: bad hex input\n");
            result.status = 1;
        } else {
            const unsigned char *command_input = opts.hex ? decoded.data : in;
            size_t command_input_len = opts.hex ? decoded.len : in_len;
            result.status = command->run(&opts, command_input, command_input_len, &out, &err);
        }
    }

    tf_buf_free(&decoded);
    if (result.status != 0) {
        tf_buf_free(&out);
    }
    result.out = out.data;
    result.out_len = out.len;
    result.err = err.data;
    result.err_len = err.len;
    return result;
}

int tf_reads_stdin(int argc, const char *const *argv) {
    const tf_command *command = NULL;
    tf_options opts;
    return validate_command(argc, argv, &command, &opts, NULL);
}

void tf_result_free(tf_result *result) {
    if (result == NULL) {
        return;
    }
    tf_buf out = {result->out, result->out_len, result->out_len};
    tf_buf err = {result->err, result->err_len, result->err_len};
    tf_buf_free(&out);
    tf_buf_free(&err);
    result->out = NULL;
    result->out_len = 0;
    result->err = NULL;
    result->err_len = 0;
    result->status = 0;
}
