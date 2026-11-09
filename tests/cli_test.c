#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(NOARGS, "", 2, "", "usage: tinyflate crc32 [--hex]\n       tinyflate adler32 [--hex]\n       tinyflate inflate [--format raw|gzip|zlib] [--hex]\n       tinyflate deflate [--format raw|gzip|zlib] [--level stored|fixed] [--hex]\n");
    failed += row(ARGS("help"), "", 0, "usage: tinyflate crc32 [--hex]\n       tinyflate adler32 [--hex]\n       tinyflate inflate [--format raw|gzip|zlib] [--hex]\n       tinyflate deflate [--format raw|gzip|zlib] [--level stored|fixed] [--hex]\n", "");
    failed += row(ARGS("-h", "x"), "", 0, "usage: tinyflate crc32 [--hex]\n       tinyflate adler32 [--hex]\n       tinyflate inflate [--format raw|gzip|zlib] [--hex]\n       tinyflate deflate [--format raw|gzip|zlib] [--level stored|fixed] [--hex]\n", "");
    failed += row(ARGS("--help", "crc32"), "", 0, "usage: tinyflate crc32 [--hex]\n       tinyflate adler32 [--hex]\n       tinyflate inflate [--format raw|gzip|zlib] [--hex]\n       tinyflate deflate [--format raw|gzip|zlib] [--level stored|fixed] [--hex]\n", "");
    failed += row(ARGS("--version"), "", 0, "tinyflate 0.1.0\n", "");
    failed += row(ARGS("--version", "x"), "", 0, "tinyflate 0.1.0\n", "");
    failed += row(ARGS("frob"), "", 2, "", "tinyflate: unknown command frob\n");
    failed += row(ARGS("Crc32"), "", 2, "", "tinyflate: unknown command Crc32\n");
    failed += row(ARGS(""), "", 2, "", "tinyflate: unknown command \n");
    failed += row(ARGS("crc32", "foo"), "", 2, "", "tinyflate: unexpected argument foo\n");
    failed += row(ARGS("adler32", "-"), "", 2, "", "tinyflate: unexpected argument -\n");
    failed += row(ARGS("crc32", "--format", "raw"), "", 2, "", "tinyflate: unknown option --format\n");
    return failed != 0;
}
