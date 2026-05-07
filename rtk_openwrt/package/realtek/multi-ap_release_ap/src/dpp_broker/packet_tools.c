#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packet_tools.h"

void * wpabuf_put(struct wpabuf *buf, size_t len)
{
	void *tmp = buf->buf + buf->used;
	buf->used += len;
	if (buf->used > buf->size) {
		printf("wpabuf %p (size=%lu used=%lu) overflow len=%lu\n",
		   buf, (unsigned long) buf->size, (unsigned long) buf->used,
		   (unsigned long) len);
	}
	return tmp;
}

void wpa_hexdump(const char *title, const uint8_t *buf,
			 size_t len)
{
	size_t i;

	printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (buf == NULL) {
		printf(" [NULL]");
	} else {
		for (i = 0; i < len; i++)
			printf(" %02x", buf[i]);
	}

	printf("\n");
}

struct wpabuf * wpabuf_alloc(size_t len)
{
	struct wpabuf *buf = malloc(sizeof(struct wpabuf) + len);
	memset(buf, 0, sizeof(struct wpabuf) + len);

	if (buf == NULL)
		return NULL;

	buf->size = len;
	buf->buf = (uint8_t *) (buf + 1);
	return buf;
}

void wpabuf_free(struct wpabuf *buf)
{
	if (buf == NULL)
		return;
	free(buf);
}
