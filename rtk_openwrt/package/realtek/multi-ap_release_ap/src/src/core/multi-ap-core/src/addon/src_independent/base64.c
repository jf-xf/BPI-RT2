/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "base64.h"

static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char base64_url_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static unsigned char *base64_gen_encode(const unsigned char *src, size_t len, size_t *out_len, const unsigned char *table, int add_pad)
{
	unsigned char *      out, *pos;
	const unsigned char *end, *in;
	size_t               olen;
	int                  line_len;

	if (len >= SIZE_MAX / 4)
		return NULL;
	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	if (add_pad)
		olen += olen / 72;      /* line feeds */
	olen++;                 /* nul termination */
	if (olen < len)
		return NULL; /* integer overflow */
	out = malloc(olen);
	if (out == NULL)
		return NULL;

	end      = src + len;
	in       = src;
	pos      = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = table[(in[0] >> 2) & 0x3f];
		*pos++ = table[(((in[0] & 0x03) << 4) | (in[1] >> 4)) & 0x3f];
		*pos++ = table[(((in[1] & 0x0f) << 2) | (in[2] >> 6)) & 0x3f];
		*pos++ = table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
	}

	if (end - in) {
		*pos++ = table[(in[0] >> 2) & 0x3f];
		if (end - in == 1) {
			*pos++ = table[((in[0] & 0x03) << 4) & 0x3f];
			if (add_pad)
				*pos++ = '=';
		} else {
			*pos++ = table[(((in[0] & 0x03) << 4) |
					(in[1] >> 4)) & 0x3f];
			*pos++ = table[((in[1] & 0x0f) << 2) & 0x3f];
		}
		if (add_pad)
			*pos++ = '=';
		line_len += 4;
	}

	*pos = '\0';
	if (out_len)
		*out_len = pos - out;
	return out;
}

static unsigned char *base64_gen_decode(const unsigned char *src, size_t len, size_t *out_len, const unsigned char *table)
{
	unsigned char dtable[256], *out, *pos, block[4], tmp, *result;
	size_t        i, count, olen;
	int           pad = 0;
	size_t        extra_pad;

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(base64_table) - 1; i++)
		dtable[(unsigned char)table[i]] = (unsigned char)i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[(unsigned char)src[i]] != 0x80)
			count++;
	}

	if (count == 0)
		return NULL;
	extra_pad = (4 - count % 4) % 4;

	olen = (count + extra_pad) / 4 * 3;
	pos = out = malloc(olen);
	if (out == NULL)
		return NULL;

	count = 0;
	for (i = 0; i < len + extra_pad; i++) {
		unsigned char val;

		if (i >= len)
			val = '=';
		else
			val = src[i];
		tmp = dtable[val];
		if (tmp == 0x80)
			continue;

		if (val == '=')
			pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count  = 0;
			if (pad) {
				if (pad == 1)
					pos--;
				else if (pad == 2)
					pos -= 2;
				else {
					/* Invalid padding */
					free(out);
					return NULL;
				}
				break;
			}
		}
	}

	*out_len = pos - out;
	result = malloc(*out_len + 1);
	memset(result, 0, *out_len + 1);
	memcpy(result, out, *out_len);
	free(out);
	return result;
}

unsigned char *base64_encode(const unsigned char *src, size_t len, size_t *out_len)
{
	return base64_gen_encode(src, len, out_len, base64_table, 1);
}

unsigned char *base64_url_encode(const unsigned char *src, size_t len, size_t *out_len)
{
	return base64_gen_encode(src, len, out_len, base64_url_table, 0);
}

unsigned char *base64_decode(const unsigned char *src, size_t len, size_t *out_len)
{
	return base64_gen_decode(src, len, out_len, base64_table);
}

unsigned char *base64_url_decode(const unsigned char *src, size_t len, size_t *out_len)
{
	return base64_gen_decode(src, len, out_len, base64_url_table);
}
