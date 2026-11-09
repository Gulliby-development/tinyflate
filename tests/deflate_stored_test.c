#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "", 0, "010000ffff\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "61", 0, "010100feff61\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "616263", 0, "010300fcff616263\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "68656c6c6f20776f726c640a", 0, "010c00f3ff68656c6c6f20776f726c640a\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", 0, "012000dfff000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "303132333435363738393031323334353637383930313233343536373839303132333435363738393031323334353637383930313233343536373839", 0, "013c00c3ff303132333435363738393031323334353637383930313233343536373839303132333435363738393031323334353637383930313233343536373839\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--hex"), "00ff", 0, "010200fdff00ff\n", "");
    failed += row(ARGS("deflate", "--level", "stored", "--format", "raw", "--hex"), "54686520717569636b2062726f776e20666f78", 0, "011300ecff54686520717569636b2062726f776e20666f78\n", "");
    failed += row(ARGS("deflate", "--format", "raw", "--level", "stored", "--hex"), "61", 0, "010100feff61\n", "");
    return failed != 0;
}
