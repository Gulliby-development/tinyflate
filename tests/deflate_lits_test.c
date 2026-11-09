#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "", 0, "0300\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "61", 0, "4b0400\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "6162", 0, "4b4c0200\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "616263", 0, "4b4c4a0600\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "616263616263", 0, "4b4c4a062200\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "007f808f90ff", 0, "63a86fe89ff01f00\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "68656c6c6f2068656c6c6f2068656c6c6f", 0, "cb48cdc9c957402201\n", "");
    failed += row(ARGS("deflate", "--hex"), "62616e616e612062616e64616e61", 0, "4b4acc034205209502a401\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "616263646566616263646566", 0, "4b4c4a4e494d839000\n", "");
    failed += row(ARGS("deflate", "--level", "fixed", "--hex"), "fafbfcfdfefffafbfcfdfefffafbfcfdfeff", 0, "fbf5fbcfdf7fff914900\n", "");
    return failed != 0;
}
