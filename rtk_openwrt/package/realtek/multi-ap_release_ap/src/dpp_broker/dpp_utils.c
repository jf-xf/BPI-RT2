#include <stdio.h>
#include <stdarg.h>
#include "dpp_utils.h"
#include "packet_tools.h"

const uint8_t * dpp_get_attr(const uint8_t *buf, size_t len, uint16_t req_id, uint16_t *ret_len)
{
	uint16_t id, alen;
	const uint8_t *pos = buf, *end = buf + len;

	while (end - pos >= 4) {
		id = WPA_GET_LE16(pos);
		pos += 2;
		alen = WPA_GET_LE16(pos);
		pos += 2;
		if (alen > end - pos)
			return NULL;
		if (id == req_id) {
			*ret_len = alen;
			return pos;
		}
		pos += alen;
	}

	return NULL;
}

int dpp_check_attrs(const uint8_t *buf, size_t len)
{
	const uint8_t *pos, *end;
	int wrapped_data = 0;

	pos = buf;
	end = buf + len;
	while (end - pos >= 4) {
		uint16_t id, alen;

		id = WPA_GET_LE16(pos);
		pos += 2;
		alen = WPA_GET_LE16(pos);
		pos += 2;
		dpp_printf("DPP: Attribute ID %04x len %u",
			   id, alen);
		if (alen > end - pos) {
			dpp_printf("DPP: Truncated message - not enough room for the attribute - dropped");
			return -1;
		}
		if (wrapped_data) {
			dpp_printf("DPP: An unexpected attribute included after the Wrapped Data attribute");
			return -1;
		}
		if (id == DPP_ATTR_WRAPPED_DATA)
			wrapped_data = 1;
		pos += alen;
	}

	if (end != pos) {
		dpp_printf("DPP: Unexpected octets (%d) after the last attribute",
			   (int) (end - pos));
		return -1;
	}

	return 0;
}

uint8_t * dpp_get_tlv(uint8_t *buf, size_t len, uint16_t req_id)
{
	uint16_t id, alen;
	uint8_t *pos = buf, *end = buf + len;

	while (end - pos >= 4) {
		id = *pos;
		pos += 1;
		alen = WPA_GET_BE16(pos);
		pos += 2;
		if (alen > end - pos)
			return NULL;
		if (id == req_id) {
			return pos;
		}
		pos += alen;
	}

	return NULL;
}

void dpp_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf("DPP Broker: ");
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
}