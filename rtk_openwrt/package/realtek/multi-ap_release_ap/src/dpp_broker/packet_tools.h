#ifndef _PACKET_TOOLS_H_
#define _PACKET_TOOLS_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>

struct wpabuf {
	size_t size; /* total size of the allocated buffer */
	size_t used; /* length of data in the buffer */
	uint8_t *buf; /* pointer to the head of the buffer */
	unsigned int flags;
	/* optionally followed by the allocated buffer */
};

void wpa_hexdump(const char *title, const uint8_t *buf, size_t len);

void * wpabuf_put(struct wpabuf *buf, size_t len);

static inline uint16_t WPA_GET_LE16(const uint8_t *a)
{
	return (a[1] << 8) | a[0];
}

static inline uint16_t WPA_GET_BE16(const uint8_t *a)
{
	return (a[0] << 8) | a[1];
}

static inline int WPA_GET_BE24(const uint8_t *a)
{
	return (a[0] << 16) | (a[1] << 8) | a[2];
}

static inline int WPA_GET_BE32(const uint8_t *a)
{
	return ((int) a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}

static inline void WPA_PUT_BE32(uint8_t *a, int val)
{
	a[0] = (val >> 24) & 0xff;
	a[1] = (val >> 16) & 0xff;
	a[2] = (val >> 8) & 0xff;
	a[3] = val & 0xff;
}

static inline const void * wpabuf_head(const struct wpabuf *buf)
{
	return buf->buf;
}

static inline const uint8_t * wpabuf_head_u8(const struct wpabuf *buf)
{
	return (const uint8_t *) wpabuf_head(buf);
}

static inline void wpabuf_put_be32(struct wpabuf *buf, int data)
{
	uint8_t *pos = (uint8_t *) wpabuf_put(buf, 4);
	WPA_PUT_BE32(pos, data);
}

static inline void wpabuf_put_data(struct wpabuf *buf, const void *data,
				   size_t len)
{
	if (data)
		memcpy(wpabuf_put(buf, len), data, len);
}

static inline size_t wpabuf_tailroom(const struct wpabuf *buf)
{
	return buf->size - buf->used;
}

static inline size_t wpabuf_len(const struct wpabuf *buf)
{
	return buf->used;
}

struct wpabuf * wpabuf_alloc(size_t len);

void wpabuf_free(struct wpabuf *buf);
#endif