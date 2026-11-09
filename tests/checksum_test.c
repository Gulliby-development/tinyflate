#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("crc32"), "", 0, "00000000\n", "");
    failed += row(ARGS("crc32"), "a", 0, "e8b7be43\n", "");
    failed += row(ARGS("crc32"), "abc", 0, "352441c2\n", "");
    failed += row(ARGS("crc32"), "123456789", 0, "cbf43926\n", "");
    failed += row(ARGS("crc32"), "The quick brown fox jumps over the lazy dog", 0, "414fa339\n", "");
    failed += row(ARGS("crc32"), "\n", 0, "32d70693\n", "");
    failed += row(ARGS("crc32"), "hello world\r\n", 0, "38e6c41a\n", "");
    failed += row(ARGS("adler32"), "", 0, "00000001\n", "");
    failed += row(ARGS("adler32"), "a", 0, "00620062\n", "");
    failed += row(ARGS("adler32"), "abc", 0, "024d0127\n", "");
    failed += row(ARGS("adler32"), "123456789", 0, "091e01de\n", "");
    failed += row(ARGS("adler32"), "The quick brown fox jumps over the lazy dog", 0, "5bdc0fda\n", "");
    failed += row(ARGS("adler32"), "Wikipedia", 0, "11e60398\n", "");
    return failed != 0;
}
