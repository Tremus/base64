#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../../include/libbase64.h"
#include "../../tables/tables.h"
#include "../../codecs.h"
#include "../../env.h"

#if BASE64_HAVE_AVX512
#include <immintrin.h>

#include "../avx2/dec_reshuffle.c"
#include "../avx2/dec_loop.c"
#include "enc_reshuffle_translate.c"
#include "enc_loop.c"

#endif	// BASE64_HAVE_AVX512

void base64_stream_encode_avx512(struct base64_state *state, const char *src, size_t srclen, char	*out, size_t *outlen)
{
#if BASE64_HAVE_AVX512
	#include "../generic/enc_head.c"
	enc_loop_avx512(&s, &slen, &o, &olen);
	#include "../generic/enc_tail.c"
#else
	BASE64_ENC_STUB
#endif
}

// Reuse AVX2 decoding. Not supporting AVX512 at present
int	base64_stream_decode_avx512(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen)
{
#if BASE64_HAVE_AVX512
	#include "../generic/dec_head.c"
	dec_loop_avx2(&s, &slen, &o, &olen);
	#include "../generic/dec_tail.c"
#else
	BASE64_DEC_STUB
#endif
}
