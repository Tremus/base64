#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../../../include/libbase64.h"
#include "../../tables/tables.h"
#include "../../codecs.h"
#include "../../env.h"

#if BASE64_WORDSIZE == 32
#  include "32/enc_loop.c"
#elif BASE64_WORDSIZE == 64
#  include "64/enc_loop.c"
#endif

#if BASE64_WORDSIZE >= 32
#  include "32/dec_loop.c"
#endif

void base64_stream_encode_plain(struct base64_state *state, const char *src, size_t srclen, char	*out, size_t *outlen)
{
	#include "enc_head.c"
#if BASE64_WORDSIZE == 32
	enc_loop_generic_32(&s, &slen, &o, &olen);
#elif BASE64_WORDSIZE == 64
	enc_loop_generic_64(&s, &slen, &o, &olen);
#endif
	#include "enc_tail.c"
}

int	base64_stream_decode_plain(struct base64_state *state, const char *src, size_t srclen, char *out, size_t *outlen)
{
	#include "dec_head.c"
#if BASE64_WORDSIZE >= 32
	dec_loop_generic_32(&s, &slen, &o, &olen);
#endif
	#include "dec_tail.c"
}
