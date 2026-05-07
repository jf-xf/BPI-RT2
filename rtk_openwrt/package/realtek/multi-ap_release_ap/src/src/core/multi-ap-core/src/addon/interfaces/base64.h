/*
 * Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _BASE_64_H_
#define _BASE_64_H_

unsigned char *base64_encode(const unsigned char *src, size_t len, size_t *out_len);
unsigned char *base64_url_encode(const unsigned char *src, size_t len, size_t *out_len);

unsigned char *base64_decode(const unsigned char *src, size_t len, size_t *out_len);
unsigned char *base64_url_decode(const unsigned char *src, size_t len, size_t *out_len);

#endif