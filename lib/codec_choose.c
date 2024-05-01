#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../include/libbase64.h"
#include "codecs.h"
#include "env.h"

#if (__x86_64__ || __i386__ || _M_X86 || _M_X64)
  #define BASE64_X86
  #if (BASE64_HAVE_SSSE3 || BASE64_HAVE_SSE41 || BASE64_HAVE_SSE42 || BASE64_HAVE_AVX || BASE64_HAVE_AVX2 || BASE64_HAVE_AVX512)
    #define BASE64_X86_SIMD
  #endif
#endif

#ifdef BASE64_X86
#ifdef _MSC_VER
	#include <intrin.h>
	#define __cpuid_count(__level, __count, __eax, __ebx, __ecx, __edx) \
	{						\
		int info ## __level[4];				\
		__cpuidex(info, __level, __count);	\
		__eax = info ## __level[0];			\
		__ebx = info ## __level[1];			\
		__ecx = info ## __level[2];			\
		__edx = info ## __level[3];			\
	}
	#define __cpuid(__level, __eax, __ebx, __ecx, __edx) \
		__cpuid_count(__level, 0, __eax, __ebx, __ecx, __edx)
#else
	#include <cpuid.h>
	#if BASE64_HAVE_AVX512 || BASE64_HAVE_AVX2 || BASE64_HAVE_AVX
		#if ((__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 2) || (__clang_major__ >= 3))
			static inline uint64_t _xgetbv (uint32_t index)
			{
				uint32_t eax, edx;
				__asm__ __volatile__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(index));
				return ((uint64_t)edx << 32) | eax;
			}
		#else
			#error "Platform not supported"
		#endif
	#endif
#endif

#ifndef BASE64_bit_AVX512vl
#define BASE64_bit_AVX512vl (1 << 31)
#endif
#ifndef BASE64_bit_AVX512vbmi
#define BASE64_bit_AVX512vbmi (1 << 1)
#endif
#ifndef BASE64_bit_AVX2
#define BASE64_bit_AVX2 (1 << 5)
#endif
#ifndef BASE64_bit_SSSE3
#define BASE64_bit_SSSE3 (1 << 9)
#endif
#ifndef BASE64_bit_SSE41
#define BASE64_bit_SSE41 (1 << 19)
#endif
#ifndef BASE64_bit_SSE42
#define BASE64_bit_SSE42 (1 << 20)
#endif
#ifndef BASE64_bit_AVX
#define BASE64_bit_AVX (1 << 28)
#endif

#define BASE64_bit_XSAVE_XRSTORE (1 << 27)

#ifndef _XCR_XFEATURE_ENABLED_MASK
#define _XCR_XFEATURE_ENABLED_MASK 0
#endif

#define BASE64__XCR_XMM_AND_YMM_STATE_ENABLED_BY_OS 0x6
#endif

void base64_stream_encode_avx(struct base64_state *state, const char *src, size_t srclen, char	*out, size_t *outlen);
void base64_stream_encode_avx2(struct base64_state *state, const char *src, size_t srclen, char	*out, size_t *outlen);
void base64_stream_encode_avx512(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
void base64_stream_encode_plain(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
void base64_stream_encode_neon32(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
void base64_stream_encode_neon64(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
void base64_stream_encode_sse41(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
void base64_stream_encode_sse42(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
void base64_stream_encode_ssse3(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);

int	base64_stream_decode_avx512(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_avx2(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_neon32(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_neon64(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_plain(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_ssse3(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_sse41(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_sse42(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);
int	base64_stream_decode_avx(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen);

static bool
codec_choose_forced (struct base64_codec *codec, int flags)
{
	// If the user wants to use a certain codec,
	// always allow it, even if the codec is a no-op.
	// For testing purposes.

	if (!(flags & 0xFFFF)) {
		return false;
	}

	// if (flags & BASE64_FORCE_AVX2) {
	// 	codec->enc = base64_stream_encode_avx2;
	// 	codec->dec = base64_stream_decode_avx2;
	// 	return true;
	// }
	// if (flags & BASE64_FORCE_NEON32) {
	// 	codec->enc = base64_stream_encode_neon32;
	// 	codec->dec = base64_stream_decode_neon32;
	// 	return true;
	// }
	// if (flags & BASE64_FORCE_NEON64) {
	// 	codec->enc = base64_stream_encode_neon64;
	// 	codec->dec = base64_stream_decode_neon64;
	// 	return true;
	// }
	if (flags & BASE64_FORCE_PLAIN) {
		codec->enc = base64_stream_encode_plain;
		codec->dec = base64_stream_decode_plain;
		return true;
	}
	if (flags & BASE64_FORCE_SSSE3) {
		codec->enc = base64_stream_encode_ssse3;
		codec->dec = base64_stream_decode_ssse3;
		return true;
	}
	if (flags & BASE64_FORCE_SSE41) {
		codec->enc = base64_stream_encode_sse41;
		codec->dec = base64_stream_decode_sse41;
		return true;
	}
	if (flags & BASE64_FORCE_SSE42) {
		codec->enc = base64_stream_encode_sse42;
		codec->dec = base64_stream_decode_sse42;
		return true;
	}
	if (flags & BASE64_FORCE_AVX) {
		codec->enc = base64_stream_encode_avx;
		codec->dec = base64_stream_decode_avx;
		return true;
	}
	if (flags & BASE64_FORCE_AVX512) {
		codec->enc = base64_stream_encode_avx512;
		codec->dec = base64_stream_decode_avx512;
		return true;
	}
	return false;
}

static bool
codec_choose_arm (struct base64_codec *codec)
{
#if (defined(__ARM_NEON__) || defined(__ARM_NEON)) && ((defined(__aarch64__) && BASE64_HAVE_NEON64) || BASE64_HAVE_NEON32)

	// Unfortunately there is no portable way to check for NEON
	// support at runtime from userland in the same way that x86
	// has cpuid, so just stick to the compile-time configuration:

	#if defined(__aarch64__) && BASE64_HAVE_NEON64
	codec->enc = base64_stream_encode_neon64;
	codec->dec = base64_stream_decode_neon64;
	#else
	codec->enc = base64_stream_encode_neon32;
	codec->dec = base64_stream_decode_neon32;
	#endif

	return true;

#else
	(void)codec;
	return false;
#endif
}

static bool
codec_choose_x86 (struct base64_codec *codec)
{
#ifdef BASE64_X86_SIMD

	unsigned int eax, ebx = 0, ecx = 0, edx;
	unsigned int max_level;

	#ifdef _MSC_VER
	int info[4];
	__cpuidex(info, 0, 0);
	max_level = info[0];
	#else
	max_level = __get_cpuid_max(0, NULL);
	#endif

	#if BASE64_HAVE_AVX512 || BASE64_HAVE_AVX2 || BASE64_HAVE_AVX
	// Check for AVX/AVX2/AVX512 support:
	// Checking for AVX requires 3 things:
	// 1) CPUID indicates that the OS uses XSAVE and XRSTORE instructions
	//    (allowing saving YMM registers on context switch)
	// 2) CPUID indicates support for AVX
	// 3) XGETBV indicates the AVX registers will be saved and restored on
	//    context switch
	//
	// Note that XGETBV is only available on 686 or later CPUs, so the
	// instruction needs to be conditionally run.
	if (max_level >= 1) {
		__cpuid_count(1, 0, eax, ebx, ecx, edx);
		if (ecx & BASE64_bit_XSAVE_XRSTORE) {
			uint64_t xcr_mask;
			xcr_mask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
			if (xcr_mask & BASE64__XCR_XMM_AND_YMM_STATE_ENABLED_BY_OS) {
				#if BASE64_HAVE_AVX512
				if (max_level >= 7) {
					__cpuid_count(7, 0, eax, ebx, ecx, edx);
					if ((ebx & BASE64_bit_AVX512vl) && (ecx & BASE64_bit_AVX512vbmi)) {
						codec->enc = base64_stream_encode_avx512;
						codec->dec = base64_stream_decode_avx512;
						return true;
					}
				}
				#endif
				#if BASE64_HAVE_AVX2
				if (max_level >= 7) {
					__cpuid_count(7, 0, eax, ebx, ecx, edx);
					if (ebx & BASE64_bit_AVX2) {
						codec->enc = base64_stream_encode_avx2;
						codec->dec = base64_stream_decode_avx2;
						return true;
					}
				}
				#endif
				#if BASE64_HAVE_AVX
				__cpuid_count(1, 0, eax, ebx, ecx, edx);
				if (ecx & BASE64_bit_AVX) {
					codec->enc = base64_stream_encode_avx;
					codec->dec = base64_stream_decode_avx;
					return true;
				}
				#endif
			}
		}
	}
	#endif

	#if BASE64_HAVE_SSE42
	// Check for SSE42 support:
	if (max_level >= 1) {
		__cpuid(1, eax, ebx, ecx, edx);
		if (ecx & BASE64_bit_SSE42) {
			codec->enc = base64_stream_encode_sse42;
			codec->dec = base64_stream_decode_sse42;
			return true;
		}
	}
	#endif

	#if BASE64_HAVE_SSE41
	// Check for SSE41 support:
	if (max_level >= 1) {
		__cpuid(1, eax, ebx, ecx, edx);
		if (ecx & BASE64_bit_SSE41) {
			codec->enc = base64_stream_encode_sse41;
			codec->dec = base64_stream_decode_sse41;
			return true;
		}
	}
	#endif

	#if BASE64_HAVE_SSSE3
	// Check for SSSE3 support:
	if (max_level >= 1) {
		__cpuid(1, eax, ebx, ecx, edx);
		if (ecx & BASE64_bit_SSSE3) {
			codec->enc = base64_stream_encode_ssse3;
			codec->dec = base64_stream_decode_ssse3;
			return true;
		}
	}
	#endif

#else
	(void)codec;
#endif

	return false;
}

void
base64_codec_choose (struct base64_codec *codec, int flags)
{
	// User forced a codec:
	if (codec_choose_forced(codec, flags)) {
		return;
	}

	// Runtime feature detection:
	if (codec_choose_arm(codec)) {
		return;
	}
	if (codec_choose_x86(codec)) {
		return;
	}
	codec->enc = base64_stream_encode_plain;
	codec->dec = base64_stream_decode_plain;
}
