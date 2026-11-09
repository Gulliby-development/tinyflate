#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("deflate", "--format", "gzip", "--level", "stored", "--hex"), "", 0, "1f8b08000000000000ff010000ffff0000000000000000\n", "");
    failed += row(ARGS("deflate", "--format", "gzip", "--level", "stored", "--hex"), "616263", 0, "1f8b08000000000000ff010300fcff616263c241243503000000\n", "");
    failed += row(ARGS("deflate", "--format", "gzip", "--level", "fixed", "--hex"), "616263", 0, "1f8b08000000000000ff4b4c4a0600c241243503000000\n", "");
    failed += row(ARGS("deflate", "--format", "gzip", "--hex"), "68656c6c6f2068656c6c6f2068656c6c6f", 0, "1f8b08000000000000ffcb48cdc9c9574022018088f9e511000000\n", "");
    failed += row(ARGS("deflate", "--format", "zlib", "--level", "stored", "--hex"), "", 0, "7801010000ffff00000001\n", "");
    failed += row(ARGS("deflate", "--format", "zlib", "--level", "stored", "--hex"), "616263", 0, "7801010300fcff616263024d0127\n", "");
    failed += row(ARGS("deflate", "--format", "zlib", "--level", "fixed", "--hex"), "616263", 0, "78014b4c4a0600024d0127\n", "");
    failed += row(ARGS("deflate", "--format", "zlib", "--hex"), "68656c6c6f2068656c6c6f2068656c6c6f", 0, "7801cb48cdc9c9574022013a2e067d\n", "");
    failed += row(ARGS("deflate", "--format", "zlib", "--level", "stored", "--hex"), "57696b697065646961", 0, "7801010900f6ff57696b69706564696111e60398\n", "");
    failed += row(ARGS("deflate", "--format", "gzip", "--level", "fixed", "--hex"), "313233343536373839", 0, "1f8b08000000000000ff33343236313533b7b004002639f4cb09000000\n", "");
    return failed != 0;
}
