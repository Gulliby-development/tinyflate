#include "harness.h"

int main(void) {
    int failed = 0;
    failed += row(ARGS("inflate", "--hex"), "", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--hex"), "07", 1, "", "tinyflate: invalid block type\n");
    failed += row(ARGS("inflate", "--hex"), "010500fafa616263", 1, "", "tinyflate: bad stored length\n");
    failed += row(ARGS("inflate", "--hex"), "010500faff6162", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--hex"), "4b04", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--hex"), "1b03", 1, "", "tinyflate: bad symbol\n");
    failed += row(ARGS("inflate", "--hex"), "4b044200", 1, "", "tinyflate: bad distance\n");
    failed += row(ARGS("inflate", "--hex"), "4b043e", 1, "", "tinyflate: bad symbol\n");
    failed += row(ARGS("inflate", "--hex"), "f5009204", 1, "", "tinyflate: bad huffman table\n");
    failed += row(ARGS("inflate", "--hex"), "05009204", 1, "", "tinyflate: bad huffman table\n");
    failed += row(ARGS("inflate", "--hex"), "05000200", 1, "", "tinyflate: bad huffman table\n");
    failed += row(ARGS("inflate", "--hex"), "0dc6c90900200c04c056b6358347226a3c89e5eb6360482f4c3623fed0e79a87433eb52f184b0928328e4e784da0", 1, "", "tinyflate: unexpected end of input\n");
    failed += row(ARGS("inflate", "--hex"), "010300fcff61626300", 1, "", "tinyflate: trailing data\n");
    failed += row(ARGS("inflate", "--hex"), "4b040000", 1, "", "tinyflate: trailing data\n");
    return failed != 0;
}
