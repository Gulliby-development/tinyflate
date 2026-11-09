#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("crc32", "--hex"), "", 0, "00000000\n", "");
    failed += row(ARGS("crc32", "--hex"), "61", 0, "e8b7be43\n", "");
    failed += row(ARGS("crc32", "--hex"), "616263", 0, "352441c2\n", "");
    failed += row(ARGS("crc32", "--hex"), "61 62 63", 0, "352441c2\n", "");
    failed += row(ARGS("crc32", "--hex"), "61\n62\r\n63\t", 0, "352441c2\n", "");
    failed += row(ARGS("crc32", "--hex"), "ABCDEF", 0, "648d3d79\n", "");
    failed += row(ARGS("adler32", "--hex"), "6 1", 0, "00620062\n", "");
    failed += row(ARGS("crc32", "--hex"), "6", 1, "", "tinyflate: bad hex input\n");
    failed += row(ARGS("crc32", "--hex"), "6g", 1, "", "tinyflate: bad hex input\n");
    failed += row(ARGS("adler32", "--hex"), "0x61", 1, "", "tinyflate: bad hex input\n");
    failed += row(ARGS("inflate", "--hex"), "zz", 1, "", "tinyflate: bad hex input\n");
    failed += row(ARGS("deflate", "--hex"), "abc", 1, "", "tinyflate: bad hex input\n");
    return failed != 0;
}
