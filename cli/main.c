/* SPDX-License-Identifier: BSD-3-Clause */
#include "base64.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

#define INPUT_BLOCK_SIZE ((size_t)BASE64_CLI_BUFFER_SIZE)
#define ENCODED_BLOCK_SIZE \
    ((((size_t)BASE64_CLI_BUFFER_SIZE + 2U) / 3U) * 4U)

_Static_assert(BASE64_CLI_BUFFER_SIZE > 0 &&
               BASE64_CLI_BUFFER_SIZE % 12 == 0,
               "BASE64_CLI_BUFFER_SIZE must be positive and divisible by 12");

static const char* program_name = "base64";

static void usage(FILE* stream)
{
    fprintf(stream,
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

static int fail(const char* message)
{
    fprintf(stderr, "%s: %s\n", program_name, message);
    return 1;
}

static int fail_format(const char* format, ...)
{
    va_list arguments;
    fprintf(stderr, "%s: ", program_name);
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fputc('\n', stderr);
    return 1;
}

static int fail_system(const char* filename, int code)
{
#ifdef _WIN32
    char message[256];
    if (strerror_s(message, sizeof(message), code) != 0)
        return fail_format("%s: system error %d", filename, code);
    return fail_format("%s: %s", filename, message);
#else
    return fail_format("%s: %s", filename, strerror(code));
#endif
}

static bool write_all(const void* data, size_t size)
{
    const unsigned char* bytes = (const unsigned char*)data;
    while (size != 0U) {
        const size_t written = fwrite(bytes, 1U, size, stdout);
        if (written == 0U)
            return false;
        bytes += written;
        size -= written;
    }
    return true;
}

static bool parse_columns(const char* text, uint64_t* value)
{
    uint64_t result = 0U;
    if (*text == '-')
        return false;
    if (*text == '+')
        ++text;
    if (*text == '\0')
        return false;
    while (*text != '\0') {
        const unsigned digit = (unsigned char)*text - (unsigned)'0';
        if (digit > 9U || result > (UINT64_MAX - digit) / 10U)
            return false;
        result = result * 10U + digit;
        ++text;
    }
    *value = result;
    return true;
}

static int base64_value(unsigned char byte)
{
    if (byte >= 'A' && byte <= 'Z') return byte - 'A';
    if (byte >= 'a' && byte <= 'z') return byte - 'a' + 26;
    if (byte >= '0' && byte <= '9') return byte - '0' + 52;
    if (byte == '+') return 62;
    if (byte == '/') return 63;
    return -1;
}

static size_t compact_newlines(const unsigned char* input, size_t size,
                               unsigned char* output)
{
    unsigned char* destination = output;
    while (size != 0U) {
        const unsigned char* newline =
            (const unsigned char*)memchr(input, '\n', size);
        if (newline == NULL) {
            memcpy(destination, input, size);
            destination += size;
            break;
        }
        {
            const size_t span = (size_t)(newline - input);
            memcpy(destination, input, span);
            destination += span;
            input = newline + 1;
            size -= span + 1U;
        }
    }
    return (size_t)(destination - output);
}

typedef struct WrappedWriter {
    uint64_t width;
    uint64_t column;
} WrappedWriter;

static bool wrapped_write(WrappedWriter* writer, const char* data, size_t size)
{
    if (writer->width == 0U)
        return write_all(data, size);
    while (size != 0U) {
        uint64_t room;
        size_t count;
        if (writer->column == writer->width) {
            if (!write_all("\n", 1U))
                return false;
            writer->column = 0U;
        }
        room = writer->width - writer->column;
        count = room < size ? (size_t)room : size;
        if (!write_all(data, count))
            return false;
        data += count;
        size -= count;
        writer->column += count;
    }
    return true;
}

static bool wrapped_finish(const WrappedWriter* writer)
{
    return writer->width == 0U || writer->column == 0U ||
           write_all("\n", 1U);
}

static int encode_stream(FILE* input, uint64_t wrap)
{
    unsigned char input_buffer[BASE64_CLI_BUFFER_SIZE];
    char output_buffer[ENCODED_BLOCK_SIZE];
    WrappedWriter writer = {wrap, 0U};
    size_t pending = 0U;
    for (;;) {
        const size_t count = fread(input_buffer + pending, 1U,
                                   INPUT_BLOCK_SIZE - pending, input);
        const size_t total = pending + count;
        const bool at_eof = feof(input) != 0;
        const size_t encode_size = at_eof ? total : total - total % 3U;
        if (encode_size != 0U) {
            const size_t produced = base64_encode(
                input_buffer, encode_size, output_buffer);
            if (!wrapped_write(&writer, output_buffer, produced))
                return fail("write error");
        }
        pending = total - encode_size;
        if (pending != 0U)
            memmove(input_buffer, input_buffer + encode_size, pending);
        if (ferror(input))
            return fail("read error");
        if (at_eof)
            break;
    }
    if (!wrapped_finish(&writer))
        return fail("write error");
    return 0;
}

static bool flush_output(unsigned char* output, size_t* size)
{
    const bool result = write_all(output, *size);
    *size = 0U;
    return result;
}

static bool emit_output(unsigned char* output, size_t* size,
                        unsigned char byte)
{
    output[(*size)++] = byte;
    return *size != INPUT_BLOCK_SIZE || flush_output(output, size);
}

static int decode_stream(FILE* input, bool ignore_garbage)
{
    unsigned char input_buffer[BASE64_CLI_BUFFER_SIZE + 3U];
    unsigned char output_buffer[BASE64_CLI_BUFFER_SIZE];
    unsigned char filtered[BASE64_CLI_BUFFER_SIZE + 3U];
    unsigned char clean_carry[3];
    size_t clean_carry_size = 0U;
    size_t output_size = 0U;
    unsigned phase = 0U;
    uint32_t bits = 0U;
    bool need_second_padding = false;

    for (;;) {
        const size_t count = fread(
            input_buffer, 1U, INPUT_BLOCK_SIZE, input);
        size_t parse_count = count;

        if (clean_carry_size == 0U && count != 0U && phase == 0U &&
            !need_second_padding && count % 4U == 0U) {
            size_t produced;
            if (!flush_output(output_buffer, &output_size))
                return fail("write error");
            produced = base64_decode(input_buffer, count, output_buffer);
            if (produced != BASE64_ERROR) {
                if (!write_all(output_buffer, produced))
                    return fail("write error");
                if (ferror(input))
                    return fail("read error");
                if (feof(input))
                    break;
                continue;
            }
        }

        if ((count != 0U || clean_carry_size != 0U) && phase == 0U &&
            !need_second_padding) {
            unsigned char* compacted_data = filtered;
            size_t compacted;
            const size_t deferred = clean_carry_size;
            bool compact_ok = true;
            if (ignore_garbage) {
                const size_t current = base64_compact(input_buffer, count);
                if (deferred == 0U) {
                    compacted_data = input_buffer;
                    compacted = current;
                }
                else {
                    memcpy(filtered, clean_carry, deferred);
                    memcpy(filtered + deferred, input_buffer, current);
                    compacted = deferred + current;
                }
            }
            else {
                memcpy(filtered, clean_carry, deferred);
                compacted = deferred + compact_newlines(
                    input_buffer, count, filtered + deferred);
            }
            {
                const size_t aligned = compacted & ~(size_t)3U;
                if (aligned != 0U) {
                    size_t produced;
                    if (!flush_output(output_buffer, &output_size))
                        return fail("write error");
                    produced = base64_decode(compacted_data, aligned,
                                             output_buffer);
                    compact_ok = produced != BASE64_ERROR;
                    if (compact_ok && !write_all(output_buffer, produced))
                        return fail("write error");
                }
                if (compact_ok) {
                    clean_carry_size = compacted - aligned;
                    memcpy(clean_carry, compacted_data + aligned,
                           clean_carry_size);
                    if (ferror(input))
                        return fail("read error");
                    if (!feof(input))
                        continue;
                    if (clean_carry_size == 0U)
                        break;
                    memcpy(input_buffer, clean_carry, clean_carry_size);
                    parse_count = clean_carry_size;
                    clean_carry_size = 0U;
                }
                else if (ignore_garbage) {
                    if (compacted_data != input_buffer)
                        memcpy(input_buffer, compacted_data, compacted);
                    parse_count = compacted;
                    clean_carry_size = 0U;
                }
                else if (deferred != 0U) {
                    memmove(input_buffer + deferred, input_buffer, count);
                    memcpy(input_buffer, clean_carry, deferred);
                    parse_count += deferred;
                    clean_carry_size = 0U;
                }
            }
        }

        {
            size_t i;
            for (i = 0U; i < parse_count; ++i) {
                unsigned char byte;
                int value;
                if (phase == 0U && !need_second_padding) {
                    size_t end = i;
                    size_t fast_size;
                    while (end < parse_count &&
                           base64_value(input_buffer[end]) >= 0)
                        ++end;
                    fast_size = (end - i) & ~(size_t)3U;
                    if (fast_size >= 16U) {
                        size_t produced;
                        if (!flush_output(output_buffer, &output_size))
                            return fail("write error");
                        produced = base64_decode_unchecked(
                            input_buffer + i, fast_size, output_buffer, 0);
                        if (produced == BASE64_ERROR)
                            return fail("invalid input");
                        if (!write_all(output_buffer, produced))
                            return fail("write error");
                        i += fast_size - 1U;
                        continue;
                    }
                }
                byte = input_buffer[i];
                if (byte == '\n')
                    continue;
                value = base64_value(byte);
                if (value < 0 && byte != '=') {
                    if (ignore_garbage)
                        continue;
                    if (!flush_output(output_buffer, &output_size))
                        return fail("write error");
                    return fail("invalid input");
                }
                if (byte == '=') {
                    if (need_second_padding) {
                        need_second_padding = false;
                        phase = 0U;
                    }
                    else if (phase == 2U)
                        need_second_padding = true;
                    else if (phase == 3U)
                        phase = 0U;
                    else {
                        if (!flush_output(output_buffer, &output_size))
                            return fail("write error");
                        return fail("invalid input");
                    }
                    continue;
                }
                if (need_second_padding) {
                    if (!flush_output(output_buffer, &output_size))
                        return fail("write error");
                    return fail("invalid input");
                }
                if (phase == 0U) {
                    bits = (uint32_t)value << 18U;
                    phase = 1U;
                }
                else if (phase == 1U) {
                    bits |= (uint32_t)value << 12U;
                    if (!emit_output(output_buffer, &output_size,
                                     (unsigned char)(bits >> 16U)))
                        return fail("write error");
                    phase = 2U;
                }
                else if (phase == 2U) {
                    bits |= (uint32_t)value << 6U;
                    if (!emit_output(output_buffer, &output_size,
                                     (unsigned char)(bits >> 8U)))
                        return fail("write error");
                    phase = 3U;
                }
                else {
                    bits |= (uint32_t)value;
                    if (!emit_output(output_buffer, &output_size,
                                     (unsigned char)bits))
                        return fail("write error");
                    phase = 0U;
                }
            }
        }
        if (ferror(input))
            return fail("read error");
        if (feof(input))
            break;
    }

    if (!flush_output(output_buffer, &output_size))
        return fail("write error");
    if (phase != 0U || need_second_padding)
        return fail("invalid input");
    return 0;
}

static bool option_matches(const char* name, size_t length, const char* full)
{
    return length != 0U && strlen(full) >= length &&
           memcmp(name, full, length) == 0;
}

int main(int argc, char** argv)
{
    bool decode_mode = false;
    bool ignore_garbage = false;
    bool options = true;
    uint64_t wrap = 76U;
    const char* operands[2] = {NULL, NULL};
    size_t operand_count = 0U;
    int i;

    if (argc > 0 && argv[0] != NULL && *argv[0] != '\0') {
        const char* slash = strrchr(argv[0], '/');
        const char* backslash = strrchr(argv[0], '\\');
        const char* leaf = slash != NULL && backslash != NULL
            ? (slash > backslash ? slash : backslash)
            : (slash != NULL ? slash : backslash);
        program_name = leaf != NULL ? leaf + 1 : argv[0];
    }

    for (i = 1; i < argc; ++i) {
        const char* argument = argv[i];
        const size_t argument_length = strlen(argument);
        if (options && strcmp(argument, "--") == 0) {
            options = false;
            continue;
        }
        if (options && argument_length > 2U &&
            argument[0] == '-' && argument[1] == '-') {
            const char* name = argument + 2;
            const char* equals = strchr(name, '=');
            const size_t name_length = equals != NULL
                ? (size_t)(equals - name) : strlen(name);
            const bool has_value = equals != NULL;
            const char* value = has_value ? equals + 1 : NULL;
            if (option_matches(name, name_length, "help")) {
                if (has_value)
                    return fail("option '--help' doesn't allow an argument");
                usage(stdout);
                return 0;
            }
            if (option_matches(name, name_length, "version")) {
                if (has_value)
                    return fail("option '--version' doesn't allow an argument");
                printf("base64 (gaspardpetit) %s\n"
                       "Source: https://github.com/gaspardpetit/base64\n",
                       BASE64_CLI_VERSION);
                return 0;
            }
            if (option_matches(name, name_length, "decode")) {
                if (has_value)
                    return fail("option '--decode' doesn't allow an argument");
                decode_mode = true;
                continue;
            }
            if (option_matches(name, name_length, "ignore-garbage")) {
                if (has_value)
                    return fail(
                        "option '--ignore-garbage' doesn't allow an argument");
                ignore_garbage = true;
                continue;
            }
            if (option_matches(name, name_length, "wrap")) {
                if (!has_value) {
                    if (++i >= argc)
                        return fail("option '--wrap' requires an argument");
                    value = argv[i];
                }
                if (!parse_columns(value, &wrap))
                    return fail_format("invalid wrap size: '%s'", value);
                continue;
            }
            return fail_format("unrecognized option '%s'", argument);
        }
        if (options && argument_length > 1U && argument[0] == '-' &&
            strcmp(argument, "-") != 0) {
            size_t j;
            for (j = 1U; j < argument_length; ++j) {
                if (argument[j] == 'd')
                    decode_mode = true;
                else if (argument[j] == 'i')
                    ignore_garbage = true;
                else if (argument[j] == 'w') {
                    const char* value = argument + j + 1U;
                    if (*value == '\0') {
                        if (++i >= argc)
                            return fail("option requires an argument -- 'w'");
                        value = argv[i];
                    }
                    if (!parse_columns(value, &wrap))
                        return fail_format("invalid wrap size: '%s'", value);
                    break;
                }
                else
                    return fail_format("invalid option -- '%c'", argument[j]);
            }
            continue;
        }
        if (operand_count < 2U)
            operands[operand_count] = argument;
        ++operand_count;
    }

    if (operand_count > 1U) {
        fail_format("extra operand '%s'", operands[1]);
        usage(stderr);
        return 1;
    }

#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    {
        const char* filename = operand_count == 0U ? "-" : operands[0];
        FILE* input = stdin;
        int result;
        if (strcmp(filename, "-") != 0) {
            int open_error = 0;
#ifdef _WIN32
            open_error = fopen_s(&input, filename, "rb");
            if (open_error != 0)
                input = NULL;
#else
            input = fopen(filename, "rb");
            if (input == NULL)
                open_error = errno;
#endif
            if (input == NULL)
                return fail_system(filename, open_error);
        }

        result = decode_mode ? decode_stream(input, ignore_garbage)
                             : encode_stream(input, wrap);
        if (input != stdin && fclose(input) != 0 && result == 0)
            return fail_format("%s: close error", filename);
        if (fflush(stdout) != 0 && result == 0)
            return fail("write error");
        return result;
    }
}
