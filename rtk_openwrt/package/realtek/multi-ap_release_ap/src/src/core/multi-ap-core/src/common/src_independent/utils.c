/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, Realtek Semiconductor Corp.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */


#include "platform.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////////////////
//
void print_callback(void (*write_function)(const char *fmt, ...), const char *prefix, INT8U size, const char *name, const char *fmt, void *p)
{

	if (0 == PLATFORM_MEMCMP(fmt, "%s", 3)) {
		// Strings are printed with triple quotes surrounding them
		//
		write_function("%s%s: \"\"\"%s\"\"\"\n", prefix, name, p);
		return;
	} else {
#define FMT_LINE_SIZE 20
		char fmt_line[FMT_LINE_SIZE];

		fmt_line[0] = 0x0;

		PLATFORM_SNPRINTF(fmt_line, FMT_LINE_SIZE - 1, "%%s%%s: %s\n", fmt);
		fmt_line[FMT_LINE_SIZE - 1] = 0x0;

		if (1 == size) {
			write_function(fmt_line, prefix, name, *(INT8U *)p);

			return;
		} else if (2 == size) {
			write_function(fmt_line, prefix, name, *(INT16U *)p);

			return;
		} else if (4 == size) {
			write_function(fmt_line, prefix, name, *(INT32U *)p);

			return;
		}
	}

	// If we get here, it's either an IPv4 address or a sequence of bytes
	//
	{
#define AUX1_SIZE 2000 // Store a whole output line
#define AUX2_SIZE 20   // Store a fmt conversion

		INT16U i, j;

		char aux1[AUX1_SIZE];
		char aux2[AUX2_SIZE];

		INT16U space_left = AUX1_SIZE - 1;

		aux1[0] = 0x00;
		PLATFORM_STRNCAT(aux1, "%s%s: ", 6);

		for (i = 0; i < size; i++) {
			// Write one element to aux2
			//
			PLATFORM_SNPRINTF(aux2, AUX2_SIZE - 1, fmt, *((INT8U *)p + i));

			// Obtain its length
			//
			for (j = 0; j < AUX2_SIZE; j++) {
				if (aux2[j] == 0x00) {
					break;
				}
			}

			// 'j' contains the number of chars in "aux2"
			// Check if there is enough space left in "aux1"
			//
			//   NOTE: The "+2" is because we are going to append to "aux1"
			//         the contents of "aux2" plus a ", " string (which is
			//         three chars long)
			//         The second "+2" is because of the final "\n"
			//
			if (j + 2 + 2 > space_left) {
				// No more space left
				//
				aux1[AUX1_SIZE - 6] = '.';
				aux1[AUX1_SIZE - 5] = '.';
				aux1[AUX1_SIZE - 4] = '.';
				aux1[AUX1_SIZE - 3] = '.';
				aux1[AUX1_SIZE - 2] = 0x00;
				break;
			}

			// Append string to "aux1"
			//
			PLATFORM_STRNCAT(aux1, aux2, j);
			PLATFORM_STRNCAT(aux1, ", ", 2);
			space_left -= (j + 2);
		}

		PLATFORM_STRNCAT(aux1, "\n", 2);
		write_function(aux1, prefix, name);
	}

	return;
}

#if defined(CONFIG_RTK_PON_XDSL) || !defined(__UCLIBC__)
/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
        const char *osrc = src;
        size_t nleft = dsize;

        /* Copy as many bytes as will fit. */
        if (nleft != 0) {
                while (--nleft != 0) {
                        if ((*dst++ = *src++) == '\0')
                                break;
                }
        }

        /* Not enough room in dst, add NUL and traverse rest of src. */
        if (nleft == 0) {
                if (dsize != 0)
                        *dst = '\0';            /* NUL-terminate dst */
                while (*src++)
                        ;
        }

        return(src - osrc - 1); /* count does not include NUL */
}

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t dsize)
{
        const char *odst = dst;
        const char *osrc = src;
        size_t n = dsize;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end. */
        while (n-- != 0 && *dst != '\0')
                dst++;
        dlen = dst - odst;
        n = dsize - dlen;

        if (n-- == 0)
                return(dlen + strlen(src));
        while (*src != '\0') {
                if (n != 0) {
                        *dst++ = *src;
                        n--;
                }
                src++;
        }
        *dst = '\0';

        return(dlen + (src - osrc));    /* count does not include NUL */
}
#endif

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


static int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @bin: Buffer for the converted binary data
 * @len: Length of the hex bytes to convert
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2bin(const char *hex, char *bin, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	char *opos = bin;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

/**
 * bin2hexstr - Convert binary data to ASCII hex string
 * @bin: binary data
 * @hex: Buffer for the coverted ASCII hex string (the size must be larger than 2 * len + 1)
 * @len: Length of binary data to convert
 * Returns: 0 on success, -1 on failure
 */
int bin2hexstr(const char *bin, char *hex, size_t len)
{
	size_t i;
	char   hex_byte[3];

	hex[0] = 0;
	for (i = 0; i < len; i++) {
		PLATFORM_SNPRINTF(hex_byte, sizeof(hex_byte),
		                  "%02X", (unsigned char)bin[i]);
		PLATFORM_STRCAT(hex, hex_byte);
	}
	return 0;
}