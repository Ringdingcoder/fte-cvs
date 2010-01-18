#include "fte.h"
#include <string.h>

size_t UnTabStr(char *dest, size_t maxlen, const char *source, size_t slen) {
    const char * const end = dest + maxlen - 1;
    char *p = dest;
    unsigned pos = 0;

    for (unsigned i = 0; p < end && i < slen; ++i) {
	if (source[i] == '\t') {
	    do {
		if (p < end)
		    *p++ = ' ';
	    } while (++pos & 0x7);
	} else {
	    *p++ = source[i];
	    pos++;
	}
    }

    if (p <= end)
	*p = '\0';

    return pos;
}

size_t UnEscStr(char *dest, size_t maxlen, const char *source, size_t slen) {
    const char * const end = dest + maxlen - 1;
    char *p = dest;

    for (unsigned i = 0; p < end && i < slen; ++i) {
	if (source[i] == 0x1B) { // ESC-seq
	    if (++i < slen && (source[i] == '[')) {
		while (++i < slen &&
		       ((source[i] >= '0' && source[i] <= '9')
			|| source[i] == ';'))
		    ;
	    } else
		*p++ = '^';
	} else if (source[i] == (char)0xE2 && (i + 1) < slen && source[i + 1] == (char)0x80) {
	    // Replace localized UTF8 apostrophes used by gcc.
	    *p++ = '\'';
	    i += 2;
	} else
	    *p++ = source[i];
    }

    if (p <= end)
	*p = '\0';

    return p - dest;
}

#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }

    return ret;
}
#endif // !HAVE_STRLCPY

#if !defined(HAVE_STRLCAT)
size_t strlcat(char *dst, const char *src, size_t size)
{
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);

    if (size) {
        size_t len = (src_len >= size-dst_len) ? (size-dst_len-1) : src_len;
        memcpy(&dst[dst_len], src, len);
        dst[dst_len + len] = '\0';
    }

    return dst_len + src_len;
}
#endif // !HAVE_STRLCAT
