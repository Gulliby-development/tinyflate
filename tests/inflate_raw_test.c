#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("inflate", "--hex"), "010000ffff", 0, "\n", "");
    failed += row(ARGS("inflate", "--hex"), "010300fcff616263", 0, "616263\n", "");
    failed += row(ARGS("inflate", "--hex"), "000200fdff6869010100feff21", 0, "686921\n", "");
    failed += row(ARGS("inflate", "--hex"), "0300", 0, "\n", "");
    failed += row(ARGS("inflate", "--hex"), "4b0400", 0, "61\n", "");
    failed += row(ARGS("inflate", "--hex"), "cb48cdc9c957402201", 0, "68656c6c6f2068656c6c6f2068656c6c6f\n", "");
    failed += row(ARGS("inflate", "--hex"), "4b840300", 0, "61616161616161616161\n", "");
    failed += row(ARGS("inflate", "--hex"), "ebe9edeb9f3071d2e42953a74d9f3173d6ec3973e7cd0700", 0, "8c8d8e8f909192939495969798999a9b9c9d9e9f\n", "");
    failed += row(ARGS("inflate", "--hex"), "0dc6c90900200c04c056b6358347226a3c89e5eb6360482f4c3623fed0e79a87433eb52f184b0928328e4e784da0a9d61e", 0, "626f78207769746820666f7820626f7820616e642061206a756d7073207768696c65206c6971756f7220646f672062726f776e\n", "");
    failed += row(ARGS("inflate", "--hex"), "0dc8d91100100c05c0565e6d8e218e09218eeed9cf2dd49505490336cd085e5ed0956c2ef76f6d038e0366f4307c50ef03", 0, "6c6971756f72206a75672077697468206f76657220717569636b6c79206a756d707320646f672074686520626f78206d79\n", "");
    failed += row(ARGS("inflate", "--format", "raw", "--hex"), "010300FCFF616263", 0, "616263\n", "");
    failed += row(ARGS("inflate", "--hex"), "0dc6c909 00200c04 c056b635 8347226a 3c89e5eb 6360482f 4c3623fe d0e79a87 433eb52f 184b0928 328e4e78 4da0a9d6 1e", 0, "626f78207769746820666f7820626f7820616e642061206a756d7073207768696c65206c6971756f7220646f672062726f776e\n", "");
    return failed != 0;
}
