#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../include/libbase64.h"

// Cast away unused variable, silence compiler:
#define BASE64_UNUSED(x)        ((void)(x))

// Stub function when encoder arch unsupported:
#define BASE64_ENC_STUB                \
    BASE64_UNUSED(state);              \
    BASE64_UNUSED(src);                \
    BASE64_UNUSED(srclen);             \
    BASE64_UNUSED(out);                \
    *outlen = 0;

// Stub function when decoder arch unsupported:
#define BASE64_DEC_STUB                \
    BASE64_UNUSED(state);              \
    BASE64_UNUSED(src);                \
    BASE64_UNUSED(srclen);             \
    BASE64_UNUSED(out);                \
    BASE64_UNUSED(outlen);             \
    return -1;

struct base64_codec
{
    void (* enc) (struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
    int  (* dec) (struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
};

extern void base64_codec_choose (struct base64_codec *, int flags);