<p align="center"><img src="docs/logo.svg" width="96" alt="" /></p>

# tinyflate

[![CI](https://github.com/Gulliby-development/tinyflate/actions/workflows/ci.yml/badge.svg)](https://github.com/Gulliby-development/tinyflate/actions/workflows/ci.yml) [![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

tinyflate is a portable C11 command-line filter for inspecting and producing DEFLATE streams (RFC 1951), gzip files (RFC 1952) and zlib streams (RFC 1950). It also calculates CRC-32 and Adler-32 checksums. It is intended for people debugging HTTP or PNG payloads, developing compressors, and investigating why a compressed stream is rejected. The decoder reports the first applicable format error.

The implementation uses the C standard library and has no external dependencies. It is designed for Windows, macOS and Linux, and builds with MSVC, gcc and clang.

## Build

CMake 3.16 or newer is required. From the repository root:

```sh
cmake -S . -B build
cmake --build build --config Debug
```

The executable is `build/tinyflate` on single-configuration generators and `build/Debug/tinyflate.exe` with the supplied Visual Studio configuration. Substitute the latter path in Windows examples below.

## Usage

```text
usage: tinyflate crc32 [--hex]
       tinyflate adler32 [--hex]
       tinyflate inflate [--format raw|gzip|zlib] [--hex]
       tinyflate deflate [--format raw|gzip|zlib] [--level stored|fixed] [--hex]
```

The default format is raw DEFLATE; the default encoding level is `fixed`. Options can appear in any order, but each option may be supplied only once. Values for `--format` are `raw`, `gzip` and `zlib`; values for `--level` are `stored` and `fixed`. The level applies only to `deflate`.

With no command, tinyflate prints the usage text to standard error and exits 2. The first word `help`, `-h` or `--help` prints usage to standard output and exits 0; `--version` prints `tinyflate 0.1.0` and exits 0. These special words ignore further arguments. Any other unrecognized command exits 2 with `tinyflate: unknown command WORD`.

Every valid command reads all of standard input. Checksum commands print eight lowercase hexadecimal digits and a line feed. `inflate` writes decoded bytes, and `deflate` writes the encoded stream. In ordinary mode these streams are binary bytes. A successful command exits 0. A decoding or input error exits 1, writes a diagnostic to standard error, and emits no standard output. Command-line errors exit 2, write one diagnostic line to standard error and do not read standard input. Standard output is reserved for successful results.

### Hex mode

`--hex` treats standard input as hexadecimal text and emits compressed or decompressed bytes as lowercase hex followed by a line feed. Checksum output is unchanged. Input digits may be uppercase or lowercase. Spaces, tabs, carriage returns and line feeds are ignored anywhere, including between the two digits of a byte. Any other character or an odd number of digits produces `tinyflate: bad hex input`, status 1, and no output. Empty encoded/decoded output in hex mode is represented by a line feed alone. Hex conversion happens after command and option validation but before command dispatch; malformed hex therefore takes precedence over a compression error.

Option words are checked from left to right. A non-option argument (including exactly `-`) is `tinyflate: unexpected argument WORD`; an unsupported option is `tinyflate: unknown option WORD`; a repeated option is `tinyflate: WORD given twice`. Missing or invalid values produce `tinyflate: --format needs one of raw, gzip, zlib` or `tinyflate: --level needs one of stored, fixed`. An option consumes its next word as its value, even when that word looks like another option. For example, `--format --hex` reports the invalid format value. Argument errors take precedence over reading or decoding input.

## Examples

The following examples use a POSIX-style shell and the built executable path for a single-configuration generator. Each `printf` supplies the exact hex text shown; the results are the bytes tinyflate writes, displayed here as hex text where applicable.

Checksums (the input bytes spell “abc”):

```sh
printf '616263' | build/tinyflate crc32 --hex
# 352441c2
printf '616263' | build/tinyflate adler32 --hex
# 024d0127
```

Decode raw, gzip and zlib streams, respectively. Each stream expands to “abc”:

```sh
printf '010300fcff616263' | build/tinyflate inflate --format raw --hex
# 616263
printf '1f8b08000000000000ff010300fcff616263c241243503000000' | build/tinyflate inflate --format gzip --hex
# 616263
printf '7801010300fcff616263024d0127' | build/tinyflate inflate --format zlib --hex
# 616263
```

Encode “abc” as raw DEFLATE with both levels, then as gzip and zlib. The stored and fixed choices are deterministic:

```sh
printf '616263' | build/tinyflate deflate --format raw --level stored --hex
# 010300fcff616263
printf '616263' | build/tinyflate deflate --format raw --level fixed --hex
# 4b4c4a0600
printf '616263' | build/tinyflate deflate --format gzip --level stored --hex
# 1f8b08000000000000ff010300fcff616263c241243503000000
printf '616263' | build/tinyflate deflate --format gzip --level fixed --hex
# 1f8b08000000000000ff4b4c4a0600c241243503000000
printf '616263' | build/tinyflate deflate --format zlib --level stored --hex
# 7801010300fcff616263024d0127
printf '616263' | build/tinyflate deflate --format zlib --level fixed --hex
# 78014b4c4a0600024d0127
```

## Decoding rules and diagnostics

A decode failure is printed as `tinyflate: TEXT` on standard error, ends in a line feed, exits 1 and produces no standard output. The decoder processes input from left to right and reports the first applicable rule. If the bytes end before a required field or code is complete, the error is `unexpected end of input`.

Raw DEFLATE is decoded block by block. The reserved block type is `invalid block type`. A stored block whose length and complement do not agree is `bad stored length`. Dynamic Huffman counts are checked immediately: HLIT plus 257 may not exceed 286, and HDIST plus 1 may not exceed 30. Oversubscribed codes produce `bad huffman table`; the code-length code must be complete, while literal/length and distance codes may be incomplete only if they have no used symbol or exactly one used symbol of length one. The code-length, literal/length and distance tables are validated in that order. A repeat code 16 without a preceding length, a repeat that runs past the requested lengths, or a literal/length table with no end-of-block symbol also produces `bad huffman table`. Reserved fixed or dynamic symbols, or a bit pattern that matches no symbol in an accepted incomplete table, produce `bad symbol`. A back-reference farther back than the output produced so far is `bad distance`.

Unused bits at the end of the final DEFLATE byte are ignored. Raw format rejects any following byte as `trailing data`.

Gzip fields are checked in this order: the two magic bytes, compression method, reserved flag bits, fixed header fields, then optional fields in flag order (extra data, zero-terminated name, zero-terminated comment, header CRC). The magic must be 1f 8b or the error is `not a gzip stream`; the method must be 8 or the error is `unsupported compression method`; any flag bit in 0xe0 gives `reserved flag bits set`. The extra field starts with a two-byte little-endian length. If the optional header CRC does not match the low 16 bits of the CRC-32 of preceding header bytes, the error is `header checksum mismatch`. After DEFLATE decoding, the eight-byte trailer CRC-32 is checked first (`crc mismatch`), then the output length modulo 2^32 (`length mismatch`); both trailer values are little-endian. Data after the trailer is `trailing data`. Only one gzip member is accepted.

For zlib, a compression-method nibble other than 8 gives `unsupported compression method`; a window-size nibble above 7 gives `unsupported window size`. Next, the two-byte header value (first byte multiplied by 256 plus the second byte) must be divisible by 31; otherwise the error is `bad zlib header check`. This check precedes the preset-dictionary flag check (`preset dictionary not supported`). The output Adler-32 trailer is big-endian; a mismatch gives `adler32 mismatch`, and bytes after it give `trailing data`.

## Encoding behavior

Stored DEFLATE splits input into blocks of at most 65,535 bytes; empty input is one final empty block. Fixed encoding uses one final fixed-Huffman block. Its greedy matcher checks distances through 32,768 and lengths through 258, permits overlapping matches, selects the smallest distance for equal-length matches, and emits a match only when its length is at least three; otherwise it emits a literal. Huffman code bits are written most-significant-bit first, extra bits least-significant-bit first, followed by the end-of-block symbol and zero padding to a byte boundary. Encoding is deterministic and does not use dynamic Huffman blocks or lazy matching.

Gzip encoding uses a fixed ten-byte header (zero flags and modification time, OS value 255), followed by the DEFLATE stream, the input CRC-32 and input length modulo 2^32 in little-endian order. Zlib encoding uses header bytes `78 01`, the DEFLATE stream and the input Adler-32 in big-endian order.

## C and library interface

The public declarations are in `src/tinyflate.h`. The raw decoder accepts a byte pointer and length, appends decoded data on success, and reports the number of compressed bytes consumed; container decoding uses that count to locate its trailer. CRC-32 and Adler-32 functions accept a running checksum, allowing callers to update a checksum incrementally. Their conventional initial values are zero for CRC-32 and one for Adler-32.

The `tf_buf` helpers append bytes or strings to a growing owned buffer and release it; empty buffers use a null data pointer. Growth and allocations are checked. `tf_result` owns the output and error buffers returned by `tf_run`; release them with `tf_result_free`. The command implementation keeps decoded output private until the complete stream and its container checks succeed.

## Development and tests

Format C and header files with clang-format:

```sh
clang-format -i src/*.c src/*.h tests/*.c tests/*.h
```

Configure, build and run the CTest suites:

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The project treats compiler warnings as errors. Tests are organized into checksum, hex I/O, command-line and option handling, raw inflate and inflate errors, gzip and zlib decoding, stored/fixed deflate, match selection and wrapper output, plus the original scaffold test.

The repository includes an MIT license in `LICENSE`.

---

Built with [BAKED](https://github.com/Gulliby-development/baked), an open-source coding agent (bench round R82).
