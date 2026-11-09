#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "789ccb48cdc9c957c84090003a2e067d", 0, "68656c6c6f2068656c6c6f2068656c6c6f\n", "");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "789c030000000001", 0, "\n", "");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "7801010300fcff616263024d0127", 0, "616263\n", "");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "78da4b4c4a4ea40d020020162275", 0, "616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263616263\n", "");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "78", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "7802", 1, "", "tinyflate: bad zlib header check\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "7701", 1, "", "tinyflate: unsupported compression method\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "8801", 1, "", "tinyflate: unsupported window size\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "7820", 1, "", "tinyflate: preset dictionary not supported\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "789ccb48cdc9c957c84090003a2e067c", 1, "", "tinyflate: adler32 mismatch\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "789ccb48cdc9c957c84090003a2e", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--format", "zlib", "--hex"), "789ccb48cdc9c957c84090003a2e067d00", 1, "", "tinyflate: trailing data\n");
    return failed != 0;
}
