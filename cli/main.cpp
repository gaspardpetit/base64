// SPDX-License-Identifier: BSD-3-Clause
#include "base64.h"

#include <array>
#include <cerrno>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#ifndef BASE64_CLI_VERSION
#define BASE64_CLI_VERSION "unknown"
#endif
#ifndef BASE64_CLI_BUFFER_SIZE
#define BASE64_CLI_BUFFER_SIZE 49152
#endif

namespace {
constexpr std::size_t input_block_size = BASE64_CLI_BUFFER_SIZE;
static_assert(input_block_size > 0U && input_block_size % 12U == 0U,
              "BASE64_CLI_BUFFER_SIZE must be positive and divisible by 12");

const char* program_name = "base64";

void usage(FILE* stream) {
    std::fprintf(stream,
        "Usage: %s [OPTION]... [FILE]\n"
        "Base64 encode or decode FILE, or standard input, to standard output.\n\n"
        "With no FILE, or when FILE is -, read standard input.\n\n"
        "Active processing backend: %s.\n\n"
        "  -d, --decode          decode data\n"
        "  -i, --ignore-garbage  when decoding, ignore non-alphabet characters\n"
        "  -w, --wrap=COLS       wrap encoded lines after COLS characters (default 76)\n"
        "                         use 0 to disable line wrapping\n"
        "      --help            display this help and exit\n"
        "      --version         output version information and exit\n",
        program_name, base64_runtime_backend());
}

int fail(const std::string& message) {
    std::fprintf(stderr, "%s: %s\n", program_name, message.c_str());
    return 1;
}

std::string system_error_message(int code) {
#ifdef _WIN32
    std::array<char, 256> buffer{};
    if (strerror_s(buffer.data(), buffer.size(), code) == 0)
        return buffer.data();
    return "system error " + std::to_string(code);
#else
    return std::strerror(code);
#endif
}

bool write_all(const void* data, std::size_t size) {
    const auto* p = static_cast<const unsigned char*>(data);
    while (size != 0U) {
        const std::size_t n = std::fwrite(p, 1, size, stdout);
        if (n == 0U) return false;
        p += n;
        size -= n;
    }
    return true;
}

bool parse_columns(std::string_view text, std::uint64_t& value) {
    if (text.empty() || text.front() == '-') return false;
    if (text.front() == '+') {
        text.remove_prefix(1);
        if (text.empty()) return false;
    }
    const char* first = text.data();
    const char* last = first + text.size();
    const auto result = std::from_chars(first, last, value, 10);
    return result.ec == std::errc{} && result.ptr == last;
}

int base64_value(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

std::size_t compact_newlines(const unsigned char* input, std::size_t size,
                             unsigned char* output) {
    unsigned char* destination = output;
    // GNU always permits LF. memchr and memcpy are vectorized by the platform
    // CRT and make the common wrapped-input path branch-light.
    while (size != 0U) {
        const auto* newline = static_cast<const unsigned char*>(
            std::memchr(input, '\n', size));
        if (!newline) {
            std::memcpy(destination, input, size);
            destination += size;
            break;
        }
        const std::size_t span = static_cast<std::size_t>(newline - input);
        std::memcpy(destination, input, span);
        destination += span;
        input = newline + 1;
        size -= span + 1U;
    }
    return static_cast<std::size_t>(destination - output);
}

class WrappedWriter {
public:
    explicit WrappedWriter(std::uint64_t width) : width_(width) {}

    bool write(const char* data, std::size_t size) {
        if (width_ == 0U) return write_all(data, size);
        while (size != 0U) {
            if (column_ == width_) {
                if (!write_all("\n", 1)) return false;
                column_ = 0;
            }
            const std::uint64_t room = width_ - column_;
            const std::size_t count = room < size
                ? static_cast<std::size_t>(room) : size;
            if (!write_all(data, count)) return false;
            data += count;
            size -= count;
            column_ += count;
        }
        return true;
    }

    bool finish() {
        return width_ == 0U || column_ == 0U || write_all("\n", 1);
    }

private:
    std::uint64_t width_;
    std::uint64_t column_ = 0;
};

int encode(FILE* input, std::uint64_t wrap) {
    std::array<unsigned char, input_block_size> in{};
    std::vector<char> out(base64_encoded_size(in.size()));
    WrappedWriter writer(wrap);
    std::size_t pending = 0;
    for (;;) {
        const std::size_t count = std::fread(
            in.data() + pending, 1, in.size() - pending, input);
        const std::size_t total = pending + count;
        const bool at_eof = std::feof(input) != 0;
        const std::size_t encode_size = at_eof ? total : total - total % 3U;
        if (encode_size != 0U) {
            const std::size_t produced = base64_encode(
                in.data(), encode_size, out.data());
            if (!writer.write(out.data(), produced)) return fail("write error");
        }
        pending = total - encode_size;
        if (pending != 0U)
            std::memmove(in.data(), in.data() + encode_size, pending);
        if (std::ferror(input)) return fail("read error");
        if (at_eof) break;
    }
    if (!writer.finish()) return fail("write error");
    return 0;
}

int decode(FILE* input, bool ignore_garbage) {
    std::array<unsigned char, input_block_size + 3U> in{};
    std::array<unsigned char, input_block_size> out{};
    std::array<unsigned char, input_block_size + 3U> filtered{};
    std::array<unsigned char, 3> clean_carry{};
    std::size_t clean_carry_size = 0;
    std::size_t out_size = 0;
    unsigned int phase = 0;
    std::uint32_t bits = 0;
    bool need_second_padding = false;

    const auto flush = [&]() {
        const bool ok = write_all(out.data(), out_size);
        out_size = 0;
        return ok;
    };
    const auto emit = [&](unsigned char byte) {
        out[out_size++] = byte;
        return out_size != out.size() || flush();
    };

    for (;;) {
        const std::size_t count = std::fread(
            in.data(), 1, input_block_size, input);
        std::size_t parse_count = count;
        // Normal unwrapped input can be validated and decoded in one SIMD
        // pass. If the codec rejects it, replay the untouched input through
        // the compatibility parser to retain GNU's whitespace, garbage,
        // padding, partial-output, and error behavior.
        if (clean_carry_size == 0U && count != 0U && phase == 0U &&
            !need_second_padding &&
            count % 4U == 0U) {
            if (!flush()) return fail("write error");
            const std::size_t produced = base64_decode(
                in.data(), count, out.data(), 0);
            if (produced != BASE64_ERROR) {
                if (!write_all(out.data(), produced)) return fail("write error");
                if (std::ferror(input)) return fail("read error");
                if (std::feof(input)) break;
                continue;
            }
        }

        if ((count != 0U || clean_carry_size != 0U) && phase == 0U &&
            !need_second_padding) {
            unsigned char* compacted_data = filtered.data();
            std::size_t compacted;
            if (ignore_garbage) {
                // Compact directly in the read buffer. With no deferred
                // quartet bytes this avoids copying the whole block.
                const std::size_t current = base64_compact(in.data(), count);
                if (clean_carry_size == 0U) {
                    compacted_data = in.data();
                    compacted = current;
                } else {
                    std::memcpy(filtered.data(), clean_carry.data(),
                                clean_carry_size);
                    std::memcpy(filtered.data() + clean_carry_size,
                                in.data(), current);
                    compacted = clean_carry_size + current;
                }
            } else {
                std::memcpy(filtered.data(), clean_carry.data(),
                            clean_carry_size);
                compacted = clean_carry_size + compact_newlines(
                    in.data(), count, filtered.data() + clean_carry_size);
            }
            const std::size_t aligned = compacted & ~(std::size_t)3U;
            bool compact_ok = true;
            if (aligned != 0U) {
                if (!flush()) return fail("write error");
                const std::size_t produced = base64_decode(
                    compacted_data, aligned, out.data(), 0);
                compact_ok = produced != BASE64_ERROR;
                if (compact_ok && !write_all(out.data(), produced))
                    return fail("write error");
            }
            if (compact_ok) {
                clean_carry_size = compacted - aligned;
                std::memcpy(clean_carry.data(), compacted_data + aligned,
                            clean_carry_size);
                if (std::ferror(input)) return fail("read error");
                if (!std::feof(input)) continue;
                if (clean_carry_size == 0U) break;
                std::memcpy(in.data(), clean_carry.data(), clean_carry_size);
                parse_count = clean_carry_size;
                clean_carry_size = 0;
            } else if (ignore_garbage) {
                // Removed controls are garbage under -i and do not affect the
                // exact parser's output or error semantics.
                if (compacted_data != in.data())
                    std::memcpy(in.data(), compacted_data, compacted);
                parse_count = compacted;
                clean_carry_size = 0;
            } else if (clean_carry_size != 0U) {
                // Restore the deferred prefix before exact replay.
                std::memmove(in.data() + clean_carry_size, in.data(), count);
                std::memcpy(in.data(), clean_carry.data(), clean_carry_size);
                parse_count += clean_carry_size;
                clean_carry_size = 0;
            }
        }

        for (std::size_t i = 0; i < parse_count; ++i) {
            // Send long, aligned runs through the optimized project codec.
            // The state machine below handles wrapping and GNU-compatible
            // partial output at malformed boundaries.
            if (phase == 0U && !need_second_padding) {
                std::size_t end = i;
                while (end < parse_count && base64_value(in[end]) >= 0) ++end;
                const std::size_t fast_size = (end - i) & ~(std::size_t)3U;
                if (fast_size >= 16U) {
                    if (!flush()) return fail("write error");
                    // The scan above admits only standard alphabet bytes;
                    // fast_size is a whole number of unpadded quanta. Avoid
                    // repeating alphabet validation inside the SIMD codec.
                    const std::size_t produced = base64_decode_unchecked(
                        in.data() + i, fast_size, out.data(), 0);
                    if (produced == BASE64_ERROR) return fail("invalid input");
                    if (!write_all(out.data(), produced)) return fail("write error");
                    i += fast_size - 1U;
                    continue;
                }
            }
            const unsigned char c = in[i];
            if (c == '\n') continue;
            const int value = base64_value(c);
            if (value < 0 && c != '=') {
                if (ignore_garbage) continue;
                if (!flush()) return fail("write error");
                return fail("invalid input");
            }

            if (c == '=') {
                if (need_second_padding) {
                    need_second_padding = false;
                    phase = 0;
                } else if (phase == 2U) {
                    need_second_padding = true;
                } else if (phase == 3U) {
                    phase = 0;
                } else {
                    if (!flush()) return fail("write error");
                    return fail("invalid input");
                }
                continue;
            }

            if (need_second_padding) {
                if (!flush()) return fail("write error");
                return fail("invalid input");
            }
            if (phase == 0U) {
                bits = static_cast<std::uint32_t>(value) << 18U;
                phase = 1;
            } else if (phase == 1U) {
                bits |= static_cast<std::uint32_t>(value) << 12U;
                if (!emit(static_cast<unsigned char>(bits >> 16U)))
                    return fail("write error");
                phase = 2;
            } else if (phase == 2U) {
                bits |= static_cast<std::uint32_t>(value) << 6U;
                if (!emit(static_cast<unsigned char>(bits >> 8U)))
                    return fail("write error");
                phase = 3;
            } else {
                bits |= static_cast<std::uint32_t>(value);
                if (!emit(static_cast<unsigned char>(bits)))
                    return fail("write error");
                phase = 0;
            }
        }
        if (std::ferror(input)) return fail("read error");
        if (std::feof(input)) break;
    }

    if (!flush()) return fail("write error");
    if (phase != 0U || need_second_padding)
        return fail("invalid input");
    return 0;
}
} // namespace

int main(int argc, char** argv) {
    if (argc > 0 && argv[0] && *argv[0]) {
        const char* slash = std::strrchr(argv[0], '/');
        const char* backslash = std::strrchr(argv[0], '\\');
        const char* leaf = slash && backslash ? (slash > backslash ? slash : backslash)
                                           : (slash ? slash : backslash);
        program_name = leaf ? leaf + 1 : argv[0];
    }

    bool decode_mode = false;
    bool ignore_garbage = false;
    std::uint64_t wrap = 76;
    std::vector<std::string> operands;
    bool options = true;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (options && arg == "--") { options = false; continue; }
        if (options && arg.starts_with("--") && arg.size() > 2U) {
            const std::size_t equals = arg.find('=');
            const std::string_view name = arg.substr(
                2, equals == std::string_view::npos ? equals : equals - 2U);
            const bool has_value = equals != std::string_view::npos;
            const std::string_view value = has_value ? arg.substr(equals + 1U)
                                                     : std::string_view{};
            const auto matches = [&](std::string_view full) {
                return !name.empty() && full.starts_with(name);
            };
            if (matches("help")) {
                if (has_value) return fail("option '--help' doesn't allow an argument");
                usage(stdout);
                return 0;
            }
            if (matches("version")) {
                if (has_value) return fail("option '--version' doesn't allow an argument");
                std::printf(
                    "base64 (gaspardpetit) %s\n"
                    "Source: https://github.com/gaspardpetit/base64\n",
                    BASE64_CLI_VERSION);
                return 0;
            }
            if (matches("decode")) {
                if (has_value) return fail("option '--decode' doesn't allow an argument");
                decode_mode = true;
                continue;
            }
            if (matches("ignore-garbage")) {
                if (has_value)
                    return fail("option '--ignore-garbage' doesn't allow an argument");
                ignore_garbage = true;
                continue;
            }
            if (matches("wrap")) {
                std::string_view columns = value;
                if (!has_value) {
                    if (++i >= argc)
                        return fail("option '--wrap' requires an argument");
                    columns = argv[i];
                }
                if (!parse_columns(columns, wrap))
                    return fail("invalid wrap size: '" + std::string(columns) + "'");
                continue;
            }
            return fail("unrecognized option '" + std::string(arg) + "'");
        }
        if (options && arg.size() > 1 && arg.front() == '-' && arg != "-") {
            for (std::size_t j = 1; j < arg.size(); ++j) {
                if (arg[j] == 'd') decode_mode = true;
                else if (arg[j] == 'i') ignore_garbage = true;
                else if (arg[j] == 'w') {
                    std::string_view value = arg.substr(j + 1);
                    if (value.empty()) {
                        if (++i >= argc) return fail("option requires an argument -- 'w'");
                        value = argv[i];
                    }
                    if (!parse_columns(value, wrap))
                        return fail("invalid wrap size: '" + std::string(value) + "'");
                    break;
                } else {
                    return fail("invalid option -- '" + std::string(1, arg[j]) + "'");
                }
            }
            continue;
        }
        operands.emplace_back(arg);
    }

    if (operands.size() > 1U) {
        fail("extra operand '" + operands[1] + "'");
        usage(stderr);
        return 1;
    }

#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    const std::string filename = operands.empty() ? "-" : operands.front();
    FILE* input = stdin;
    if (filename != "-") {
        int open_error = 0;
#ifdef _WIN32
        open_error = fopen_s(&input, filename.c_str(), "rb");
        if (open_error != 0) input = nullptr;
#else
        input = std::fopen(filename.c_str(), "rb");
        if (!input) open_error = errno;
#endif
        if (!input) return fail(filename + ": " + system_error_message(open_error));
    }

    const int result = decode_mode ? decode(input, ignore_garbage)
                                   : encode(input, wrap);
    if (input != stdin && std::fclose(input) != 0 && result == 0)
        return fail(filename + ": close error");
    if (std::fflush(stdout) != 0 && result == 0) return fail("write error");
    return result;
}
