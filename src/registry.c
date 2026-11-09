#include <stddef.h>

#include "tinyflate.h"

const tf_command tf_commands[] = {
    {"crc32", tf_cmd_crc32, 0},
    {"adler32", tf_cmd_adler32, 0},
    {"inflate", tf_cmd_inflate, TF_OPT_FORMAT},
    {"deflate", tf_cmd_deflate, TF_OPT_FORMAT | TF_OPT_LEVEL},
    {NULL, NULL, 0},
};
