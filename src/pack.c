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
                memcpy(dest + dest_i, bytes.u8, sizeof(bytes.u32));
                dest_i += sizeof(bytes.u32);
                break;
            case 'c':
            case 'b':
                //va_list char is promoted to int, so use int in va_arg
                dest[dest_i] = va_arg(arguments, int);
                dest_i += sizeof(dest[dest_i]);
                break;
            case 'f':
                //va_list float is promoted to double, so use double in va_arg
                bytes.f32 = (float) va_arg(arguments, double);
                memcpy(dest + dest_i, bytes.u8, sizeof(bytes.u8));
                dest_i += sizeof(bytes.f32);
                break;
            case 's': {
                const char *const str = va_arg(arguments, const char *);
                const size_t length = strlen(str);
                strcpy(dest + dest_i, str);
                dest_i += length + 1;
                break;
            }
            case 'a': {
                const char *const arr = va_arg(arguments, const char *);
                const uint32_t length = va_arg(arguments, uint32_t);
                bytes.u32 = length;
                memcpy(dest + dest_i, bytes.u8, length);
                dest_i += sizeof(bytes.u32);
                memcpy(dest + dest_i, arr, length);
                dest_i += length;
                break;
            }
            default: break;
        }
    }
    va_end(arguments);
}

void unpack_format(const char source[], const char *format, ...) {
    size_t source_i = 0;

    va_list arguments;
    va_start(arguments, format);
    for (; *format != '\0'; format++) {
        IntBytes32 bytes;
        switch (*format) {
            case 'u': {
                uint32_t *u32 = va_arg(arguments, uint32_t*);
                memcpy(bytes.u8, source + source_i, sizeof(bytes.u8));
                *u32 = bytes.u32;
                source_i += sizeof(*u32);
                break;
            }
            case 'd': {
                int32_t *i32 = va_arg(arguments, int32_t*);
                memcpy(bytes.u8, source + source_i, sizeof(bytes.u8));
                *i32 = bytes.i32;
                source_i += sizeof(*i32);
                break;
            }
            case 'c':
            case 'b': {
                char *chr = va_arg(arguments, char*);
                *chr = source[source_i];
                source_i += sizeof(*chr);
                break;
            }
            case 'f': {
                float *f32 = va_arg(arguments, float*);
                memcpy(bytes.u8, source + source_i, sizeof(bytes.u8));
                *f32 = bytes.f32;
                source_i += sizeof(*f32);
                break;
            }
            case 's': {
                char *str = va_arg(arguments, char *);
                strcpy(str, source + source_i);
                source_i += strlen(source + source_i) + 1;
                break;
            }
            case 'a': {
                char **const arr = va_arg(arguments, char **);
                size_t *length = va_arg(arguments, size_t *);
                memcpy(bytes.u8, source + source_i, sizeof(bytes.u8));
                source_i += sizeof(bytes.u8);

                char *buffer = malloc(bytes.u32);
                memcpy(buffer, source + source_i, bytes.u32);
                *length = bytes.u32;
                *arr = buffer;
                source_i += *length;
                break;
            }
            default: break;
        }
    }
    va_end(arguments);
}
