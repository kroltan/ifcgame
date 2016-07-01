#include "pack.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef union {
    float f32;
    uint32_t u32;
    int32_t i32;
    uint8_t u8[4];
    int8_t i8[4];
} IntBytes32;

static inline bool is_little_endian() {
    IntBytes32 endian_check;
    endian_check.u32 = 0x01020304;

    return endian_check.u8[0] != 1;
}

static inline uint32_t flip_bytes32(uint32_t value) {
    return (value & 0x000000ff) << 24
         | (value & 0x0000ff00) << 8
         | (value & 0x00ff0000) >> 8
         | (value & 0xff000000) >> 24;
}

uint32_t change_endianness(uint32_t value) {
    if (is_little_endian()) {
        return flip_bytes32(value);
    } else {
        return value;
    }
}

void pack_format(char dest[], const char *format, ...) {
    size_t dest_i = 0;

    va_list arguments;
    va_start(arguments, format);
    for (; *format != '\0'; format++) {
        IntBytes32 bytes;
        switch (*format) {
            case 'u':
            case 'd':
                bytes.u32 = va_arg(arguments, uint32_t);
                memcpy(dest + dest_i, bytes.u8, 4);
                dest_i += 4;
                break;
            case 'c':
                //va_list char is promoted to int, so use int in va_arg
                dest[dest_i] = va_arg(arguments, int);
                dest_i++;
                break;
            case 'f':
                //va_list float is promoted to double, so use double in va_arg
                bytes.f32 = va_arg(arguments, double);
                dest_i += 4;
                break;
            default: break;
        }
    }
    va_end(arguments);
}

void unpack_format(char source[], const char *format, ...) {
    size_t source_i = 0;

    va_list arguments;
    va_start(arguments, format);
    for (; *format != '\0'; format++) {
        IntBytes32 bytes;
        switch (*format) {
            case 'u': {
                uint32_t *u32 = va_arg(arguments, uint32_t*);
                memcpy(bytes.u8, source + source_i, 4);
                *u32 = bytes.u32;
                source_i += 4;
                break;
            }
            case 'd': {
                int32_t *i32 = va_arg(arguments, int32_t*);
                memcpy(bytes.u8, source + source_i, 4);
                *i32 = bytes.i32;
                source_i += 4;
                break;
            }
            case 'c':
            case 'b': {
                char *chr = va_arg(arguments, char*);
                *chr = source[source_i];
                source_i++;
                break;
            }
            case 'f': {
                float *f32 = va_arg(arguments, float*);
                memcpy(bytes.u8, source + source_i, 4);
                *f32 = bytes.f32;
                source_i += 4;
                break;
            }
            default: break;
        }
    }
    va_end(arguments);
}
