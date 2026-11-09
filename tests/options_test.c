#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("crc32", "--hex", "--hex"), "", 2, "", "tinyflate: --hex given twice\n");
    failed += row(ARGS("inflate", "--format"), "", 2, "", "tinyflate: --format needs one of raw, gzip, zlib\n");
    failed += row(ARGS("inflate", "--format", "bz2"), "", 2, "", "tinyflate: --format needs one of raw, gzip, zlib\n");
    failed += row(ARGS("deflate", "--level", "9"), "", 2, "", "tinyflate: --level needs one of stored, fixed\n");
    failed += row(ARGS("deflate", "--level"), "", 2, "", "tinyflate: --level needs one of stored, fixed\n");
    failed += row(ARGS("inflate", "--level", "stored"), "", 2, "", "tinyflate: unknown option --level\n");
    failed += row(ARGS("deflate", "--format", "gzip", "--format", "zlib"), "", 2, "", "tinyflate: --format given twice\n");
    failed += reads(ARGS("crc32"), 1);
    failed += reads(ARGS("deflate", "--hex", "--level", "stored"), 1);
    failed += reads(ARGS("deflate", "--level", "9"), 0);
    failed += reads(ARGS("help"), 0);
    return failed != 0;
}
