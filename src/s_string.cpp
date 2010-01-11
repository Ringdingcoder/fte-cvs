#include "fte.h"
#include <string.h>

int UnTabStr(char *dest, int maxlen, const char *source, int slen) {
    char *p = dest;
    int i;
    int pos = 0;

    maxlen--;
    for (i = 0; i < slen; i++) {
        if (maxlen > 0) {
            if (source[i] == '\t') {
                do {
                    if (maxlen > 0) {
                        *p++ = ' ';
                        maxlen--;
                    }
                    pos++;
                } while (pos & 0x7);
            } else {
                *p++ = source[i];
                pos++;
                maxlen--;
            }
        } else
            break;
    }

    //dest[pos] = 0;
    *p = '\0';
    return pos;
}

int UnEscStr(char *dest, int maxlen, const char *source, int slen) {
    char *p = dest;
    int i;
    int pos = 0;

    maxlen--;
    for (i = 0; i < slen; i++) {
        if (maxlen > 0) {
            if (source[i] == 0x1B) { // ESC-seq
                if (i + 1 < slen) {
                    i++;
                    if (source[i] == '[')
                    {
                      i++;
                      while( i < slen &&
                            ((source[i] >= '0' && source[i] <= '9')
                            || source[i] == ';'))
                      {
                        i++;
                      }
                    }
                    else
                    {
                        *p++ = '^';
                        pos++;
                        maxlen--;
                    }
                }
                else
                {
                    *p++ = '^';
                    pos++;
                    maxlen--;
                }
            } else if (source[i] == (char)0xE2 && i+2 < slen && source[i+1] == (char)0x80) { // Spec symbol used by gcc. Hide it and next char
                i += 2;
            } else {
                *p++ = source[i];
                pos++;
                maxlen--;
            }
        } else
            break;
    }

    //dest[pos] = 0;
    *p = '\0';
    return pos;
}

#if !defined(HAVE_STRLCPY)
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size-1 : ret;
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
