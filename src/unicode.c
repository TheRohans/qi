/**
 * Modified from the example found here:
 *  https://rosettacode.org/wiki/UTF-8_encode_and_decode#C
 * Content is available under GNU Free Documentation License 1.2
 */
#include "unicode.h"
#include "log.h"

//////////////////////
// This file is currently not being used
// trying to make an easier to understand 
// utf8 input
/////////////////////

typedef struct {
	char mask;       /* char data will be bitwise AND with this */
	char lead;       /* start bytes of current char in utf-8 encoded character */
	uint32_t beg;    /* beginning of codepoint range */
	uint32_t end;    /* end of codepoint range */
	int bits_stored; /* the number of bits from the codepoint that fits in char */
}utf_t;

utf_t * utf[] = {
	/*             mask        lead        beg      end       bits */
	[0] = &(utf_t){0b00111111, 0b10000000, 0,       0,        6    },
	[1] = &(utf_t){0b01111111, 0b00000000, 0000,    0177,     7    },
	[2] = &(utf_t){0b00011111, 0b11000000, 0200,    03777,    5    },
	[3] = &(utf_t){0b00001111, 0b11100000, 04000,   0177777,  4    },
	[4] = &(utf_t){0b00000111, 0b11110000, 0200000, 04177777, 3    },
	      &(utf_t){0},
};

/**
 * Get the number of bytes contained within this rune
 * see table above
 */
int codepoint_len(const rune cp)
{
	int len = 0;
	for(utf_t **u = utf; *u; ++u) {
		if((cp >= (*u)->beg) && (cp <= (*u)->end)) {
			break;
		}
		++len;
	}
	
	QASSERT(len <= 4);

	return len;
}
 
/**
 * Using the first char of a utf8 char, get the length
 * lengths are in the number of bytes.
 * For example:
 *  1110xxxxx would return 3
 */
int utf8_len(const char ch)
{
	int len = 0;
	for(utf_t **u = utf; *u; ++u) {
		if((ch & ~(*u)->mask) == (*u)->lead) {
			break;
		}
		++len;
	}
	
	// Malformed leading byte
	QASSERT(len <= 4);

	return len;
}

/**
 * Take a rune and turn it into an array of chars
 * that can be used as a %s kind of output. The max
 * the buffer can be is 5 (adds a null \0);
 */
void to_utf8(char *buff, const rune cp)
{
	char* ret = buff;
	const int bytes = codepoint_len(cp);
 
	int shift = utf[0]->bits_stored * (bytes - 1);
	ret[0] = (cp >> shift & utf[bytes]->mask) | utf[bytes]->lead;
	shift -= utf[0]->bits_stored;
	for(int i = 1; i < bytes; ++i) {
		ret[i] = (cp >> shift & utf[0]->mask) | utf[0]->lead;
		shift -= utf[0]->bits_stored;
	}
	ret[bytes] = '\0';
}

/**
 * Given a set of chars (max 4) turn that into a rune. Meaning,
 * look at the first bits of the chars and figure out if they are
 * continuation chars or starter chars and make a rune out of it
 * (if valid).
 */
rune to_rune(const char chr[4])
{
	int bytes = utf8_len(*chr);
	int shift = utf[0]->bits_stored * (bytes - 1);
	rune codep = (*chr++ & utf[bytes]->mask) << shift;
 
	for(int i = 1; i < bytes; ++i, ++chr) {
		shift -= utf[0]->bits_stored;
		codep |= ((char)*chr & utf[0]->mask) << shift;
	}
 
	return codep;
}

/**
 * Given an array of chars, create a utf8 array of runes.
 * In other words, given an array of bytes that might have
 * utf8 chars in it, return an array of integers that can
 * be used for display.
 *
 * Returns the new array length as it might be different from
 * the initial src_size.
 */
int str_to_utf8(const char *str, rune *dest, int src_size)
{
    char tmp[5] = {0};
    int srci = 0;
    int len = 0;
    for(int i = 0; i < src_size; i++) {
    	if(str[srci] == 0) break;
    	
    	// look at the first char to see if it's
    	// more than one byte
    	int plen = utf8_len(str[srci]);
    	if(plen > 1) {
    		// turn multibyte into a single int
    		for(int c = 0; c < plen; c++) {
    			tmp[c] = str[srci+c];
    		}
			rune r = to_rune(tmp);
			dest[len] = r;
			len++;
    	} else {
    		// one byte
	    	dest[len] = str[srci];
   	    	len++;
    	}
   	    // move the source pointer based on 
		// the number of utf8 bytes.
    	srci += plen;
    }
    
    return len;
}

/**
 * Used when flushing a fragment to the display.
 * TODO: understand this better - might not be needed
 * (from old unicode_join.c file)
 */
int unicode_to_glyphs(unsigned int *dst, unsigned int *char_to_glyph_pos,
                      int dst_size, unsigned int *src, int src_size, int reverse)
{
    int len, i;

    len = src_size;
    if (len > dst_size)
        len = dst_size;
    memcpy(dst, src, len * sizeof(unsigned int));
    if (char_to_glyph_pos) {
        for (i = 0; i < len; i++)
            char_to_glyph_pos[i] = i;
    }
    return len;
}


