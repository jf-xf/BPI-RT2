#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "map_vendor_api.h"
#include "vendor_header.h"

////////////////////////////////////
// Callback function
////////////////////////////////////

void obtain_tlv_content(uint8_t *tlv)
{
	struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)tlv;

	vendor_tlv->tlv_type = TLV_TYPE_VENDOR_SPECIFIC;
	memcpy(vendor_tlv->vendor_oui, vendor_oui, 3);

	uint8_t  payload_2[128] = { 0 };
	uint8_t  dummy_data[6]  = { 0x16, 0x15, 0x14, 0x13, 0x12, 0x11 };
	uint8_t *packet_ptr     = payload_2;
	*packet_ptr             = VENDOR_CUSTOM_TLV_TYPE_2;
	packet_ptr++;
	insert_length(6, &packet_ptr);
	memcpy(packet_ptr, dummy_data, 6);

	uint16_t payload_len = 6 + 3;

	vendor_tlv->m_nr = payload_len;
	vendor_tlv->m    = malloc(sizeof(uint8_t) * vendor_tlv->m_nr);
	memcpy(vendor_tlv->m, payload_2, vendor_tlv->m_nr);
}

void vendor_tlv_handler(uint8_t **tlvs)
{
	uint8_t *p;
	int      i = 0, j = 0;
	while (NULL != (p = tlvs[i])) {
		switch (*p) {
		case TLV_TYPE_VENDOR_SPECIFIC: {
			struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)p;
			if (0 == memcmp(vendor_oui, vendor_tlv->vendor_oui, 3)) {
				if (vendor_tlv->m_nr < 3) {
					break;
				}
				if (VENDOR_CUSTOM_TLV_TYPE_2 == vendor_tlv->m[0]) {
					printf("Message received!\n");
					for (j = 0; j < vendor_tlv->m_nr; j++) {
						printf("%02x ", vendor_tlv->m[j]);
					}
					printf("\n");
				}
			} else {
				printf("##############################################################################\n");
				printf("Unknown_vendor_oui, skip!!!\n");
			}
			break;
		}
		default: {
			printf("##############################################################################\n");
			printf("Unknown TLV, skip!!!\n");
			break;
		}
		}
		i++;
	}
}

////////////////////////////////////
// Interface implementation
////////////////////////////////////

void vendor_register_message_receive_call_back_functions(uint8_t device_role)
{
	(void)device_role;
}

void vendor_register_append_tlv_call_back_functions(uint8_t device_role)
{
	if (device_role == MAP_ROLE_AGENT)
		register_append_vendor_tlv_callback_function(obtain_tlv_content, NULL);
	else if (device_role == MAP_ROLE_CONTROLLER)
		register_append_vendor_tlv_callback_function(NULL, vendor_tlv_handler);
}