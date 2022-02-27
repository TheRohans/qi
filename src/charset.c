/*
 * Basic Charset functions for QEmacs
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "qe.h"

QECharset *first_charset = NULL;

/* specific tables */
static unsigned short table_idem[256];
static unsigned short table_utf8[256];

unsigned char utf8_length[256];

static const unsigned int min_code[7] = {
    0, 0, 0x80, 0x800, 0x10000, 0x00200000, 0x04000000,
};

static const unsigned char first_code_mask[7] = {
    0, 0, 0x1f, 0xf, 0x7, 0x3, 0x1,
};

void charset_init(void)
{
    memset(utf8_length, 1, 256);

    int i = 0xc0;
    int l = 2;
    while (l <= 6) {
        int n = first_code_mask[l] + 1;
        while (n > 0) {
            utf8_length[i++] = l;
            n--;
        }
        l++;
    }

    for (i = 0; i < 256; i++)
        table_idem[i] = i;

    /* utf8 table */
    for (i = 0; i < 256; i++)
        table_utf8[i] = INVALID_CHAR;
    for (i = 0; i < 0x80; i++)
        table_utf8[i] = i;
    for (i = 0xc0; i < 0xfe; i++)
        table_utf8[i] = ESCAPE_CHAR;

    qe_register_charset(&charset_utf8);
}

void qe_register_charset(QECharset *charset)
{
    QECharset **pp;

    pp = &first_charset;
	
    while (*pp != NULL) 
        pp = &(*pp)->next;

    *pp = charset;
}

/********************************************************/
/* UTF8 */

/**
 * return the utf8 char and increment the 'p' pointer by the
 * number of bytes we are taking to make the utf8 char.
 *
 * strict decoding is done (refuse non canonical UTF8) 
 */
int utf8_decode(const char **pp)
{
    unsigned int c, c1;
    const unsigned char *p;
    int i, l;

    p = *(const unsigned char**)pp;
    c = *p++;
    if (c <= 127) {
        // fast case for ASCII single byte character
    } else {
//		l = utf8_len(c);
//		
//		QASSERT(l < 5);
//		
//		char *uc = malloc(sizeof(char)*l);		
//		uc[0] = c;
//		for (i = 1; i <= l; i++) {
//			char c2 = *p; //(i+1);
//            uc[i] = c2;
//			p++;
//        }
//		
//		c = to_rune(uc);
//		free(uc);
		
        l = utf8_length[c];
        if (l == 1)
			// can only be multi byte code here
            goto fail;

        c = c & first_code_mask[l];
        for (i = 1; i < l; i++) {
            c1 = *p;
            if (c1 < 0x80 || c1 >= 0xc0)
                goto fail;
            p++;
            c = (c << 6) | (c1 & 0x3f);
        }
		
        if (c < min_code[l])
            goto fail;
		
        // exclude surrogate pairs and special codes
        if ((c >= 0xd800 && c <= 0xdfff) 
			|| c == 0xfffe 
			|| c == 0xffff)
            goto fail;
    }
	
	// returns the char and moves the pointer up by one
    *(const unsigned char**)pp = p;
    return c;

fail:
    *(const unsigned char**)pp = p;
    return INVALID_CHAR;
}

/**
 * NOTE: the buffer must be at least 6 bytes long. Return 
 * the position of the next char. 
 * 
 * TODO: replace this with the code in unicode.c
 */
char *utf8_encode(char *q, int c)
{
    if (c < 0x80) {
        *q++ = c;
    } else {
        if (c < 0x800) {
            *q++ = (c >> 6) | 0xc0;
        } else {
            if (c < 0x10000) {
                *q++ = (c >> 12) | 0xe0;
            } else {
                if (c < 0x00200000) {
                    *q++ = (c >> 18) | 0xf0;
                } else {
                        if (c < 0x04000000) {
                            *q++ = (c >> 24) | 0xf8;
                        } else {
                            *q++ = (c >> 30) | 0xfc;
                            *q++ = ((c >> 24) & 0x3f) | 0x80;
                        }
                        *q++ = ((c >> 18) & 0x3f) | 0x80;
                }
                *q++ = ((c >> 12) & 0x3f) | 0x80;
            }
            *q++ = ((c >> 6) & 0x3f) | 0x80;
        }
        *q++ = (c & 0x3f) | 0x80;
    }
    return q;
}

/**
 * TODO: replace this with the code in unicode.c
 */
int utf8_to_unicode(unsigned int *dest, int dest_length, 
                    const char *str)
{
    const char *p;
    unsigned int *uq, *uq_end, c;

    if (dest_length <= 0)
        return 0;

    p = str;
    uq = dest;
    uq_end = dest + dest_length - 1;
    for (;;) {
        if (uq >= uq_end)
            break;
        c = utf8_decode(&p);
        if (c == '\0')
            break;
        *uq++ = c;
    }
    *uq = 0;
    return uq - dest;
}

static void decode_utf8_init(CharsetDecodeState *s)
{
    s->table = table_utf8;
}

static int decode_utf8_func(CharsetDecodeState *s, const unsigned char **pp)
{
    return utf8_decode((const char **)(void *)pp);
}

/**
 * TODO: replace this with the code in unicode.c
 */
unsigned char *encode_utf8(QECharset *charset, unsigned char *q, int c)
{
	return (unsigned char*)utf8_encode((char*)q, c);
}

static const char *aliases_utf_8[] = { "utf8", NULL };

QECharset charset_utf8 = {
    "utf-8",
    aliases_utf_8,
    decode_utf8_init,
    decode_utf8_func,
    encode_utf8,
};

/********************************************************/
/* generic charset functions */

QECharset *find_charset(const char *str)
{
    QECharset *p;
    const char **pp;
    for (p = first_charset; p != NULL; p = p->next) {
        if (!strcasecmp(str, p->name))
            return p;
        pp = p->aliases;
        if (pp) {
            for (; *pp != NULL; pp++) {
                if (!strcasecmp(str, *pp))
                    return p;
            }
        }
    }
    return NULL;
}

void charset_decode_init(CharsetDecodeState *s, QECharset *charset)
{
    unsigned short *table;
    
    s->table = NULL; /* fail safe */
    if (charset->table_alloc) {
        table = malloc(256 * sizeof(unsigned short));
        if (!table) {
            charset = &charset_utf8; // &charset_8859_1;
        } else {
            s->table = table;
        }
    }
    s->charset = charset;
    s->decode_func = charset->decode_func;
    if (charset->decode_init)
        charset->decode_init(s);
}

void charset_decode_close(CharsetDecodeState *s)
{
    if (s->charset->table_alloc &&
        s->charset != &charset_utf8) // &charset_8859_1)
        free(s->table);
    /* safety */
    memset(s, 0, sizeof(CharsetDecodeState));
}

/* detect the charset. Actually only UTF8 is detected */
QECharset *detect_charset (const unsigned char *buf, int size)
{
	return &charset_utf8;
}

/**
 * the function uses '?' to indicate that no match could be found in
 * current charset 
 * 
 * TODO: replace with the code in unicode.c
 */
int unicode_to_charset(char *buf, unsigned int c, QECharset *charset)
{
    // to_utf8(buf, c);
	// return 1;
	
    char *q = (char *)charset->encode_func(charset, (unsigned char*)buf, c);
    if (!q) {
        q = buf;
        *q++ = '?';
    }
    *q = '\0';
    return q - buf;
}
