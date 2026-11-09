#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b0800010203040003cb48cdc9c957c84090008088f9e511000000", 0, "68656c6c6f2068656c6c6f2068656c6c6f\n", "");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b080001020304000303000000000000000000", 0, "\n", "");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b081e01020304000302007879612e747874006e6f746500ff014b4c4a0600c241243503000000", 0, "616263\n", "");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b08080102030400036e004b4c4a0600c241243503000000", 0, "616263\n", "");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8c", 1, "", "tinyflate: not a gzip stream\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b09", 1, "", "tinyflate: unsupported compression method\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b0820010203040003cb48cdc9c957c84090008088f9e511000000", 1, "", "tinyflate: reserved flag bits set\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b081e01020304000302007879612e747874006e6f74650000014b4c4a0600c241243503000000", 1, "", "tinyflate: header checksum mismatch\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b0800010203040003cb48cdc9c957c84090008188f9e511000000", 1, "", "tinyflate: crc mismatch\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b0800010203040003cb48cdc9c957c84090008088f9e511000001", 1, "", "tinyflate: length mismatch\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b0800010203040003cb48cdc9c957c84090008088f9e511", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--format", "gzip", "--hex"), "1f8b0800010203040003cb48cdc9c957c84090008088f9e51100000000", 1, "", "tinyflate: trailing data\n");
    return failed != 0;
}
