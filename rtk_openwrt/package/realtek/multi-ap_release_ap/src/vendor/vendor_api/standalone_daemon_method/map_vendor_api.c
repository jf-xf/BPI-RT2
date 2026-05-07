#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "map_vendor_api.h"
#include "vendor_header.h"

////////////////////////////////////
// Callback function
////////////////////////////////////

void controller_vendor_tlv_handler(uint8_t **tlvs)
{
	uint8_t *p;
	int      i = 0;
	mqd_t mq;
	
	while (NULL != (p = tlvs[i])) {
		switch (*p) {
		case TLV_TYPE_VENDOR_SPECIFIC: {
			struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)p;
			if (0 == memcmp(vendor_oui, vendor_tlv->vendor_oui, 3)) {
				if (vendor_tlv->m_nr < 3) {
					break;
				}
				if (VENDOR_CUSTOM_TLV_TYPE_1 == vendor_tlv->m[0]) {
                                        //printf("recv RTIC MAP message %u\n", vendor_tlv->m_nr);

                                        if ((mqd_t)-1 == (mq = mq_open("/map2rtic_que", O_WRONLY))) {
                                                printf("open map_queue fail\n");
                                        }

                                        struct mq_attr attr;
                                        mq_getattr(mq, &attr);

                                        if (mq != -1) {
                                                if(attr.mq_curmsgs >= 3) {
                                                        printf("mq_curmsgs exceed 3\n");
                                                }

                                                if (0 != mq_send(mq, (char *) vendor_tlv->m, vendor_tlv->m_nr, 0)) {
                                                        printf("mq_send error\n");
                                                }
						mq_close(mq);
                                        }

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

void agent_vendor_tlv_handler(uint8_t **tlvs)
{
	uint8_t *p;
	int      i = 0, j = 0;
	mqd_t mq;
	while (NULL != (p = tlvs[i])) {
		switch (*p) {
		case TLV_TYPE_VENDOR_SPECIFIC: {
			struct vendorSpecificTLV *vendor_tlv = (struct vendorSpecificTLV *)p;
			if (vendor_tlv->m_nr < 3) {
				break;
			}
			if (0 == memcmp(vendor_oui, vendor_tlv->vendor_oui, 3)) {
				if (VENDOR_CUSTOM_TLV_TYPE_1 == vendor_tlv->m[0]) {
                                        //printf("recv RTIC MAP message %u\n", vendor_tlv->m_nr);

                                        if ((mqd_t)-1 == (mq = mq_open("/map2rtic_que", O_WRONLY))) {
                                                printf("open map_queue fail\n");
                                        }

                                        struct mq_attr attr;
                                        mq_getattr(mq, &attr);

                                        if (mq != -1) {
                                                if(attr.mq_curmsgs >= 3) {
                                                        printf("mq_curmsgs exceed 3\n");
                                                }

                                                if (0 != mq_send(mq, (char *) vendor_tlv->m, vendor_tlv->m_nr, 0)) {
                                                        printf("mq_send error\n");
                                                }
						mq_close(mq);
                                        }

				}
			} 
			
			if (VENDOR_CUSTOM_TLV_TYPE_2 == vendor_tlv->m[0]) {
				printf("Message received!\n");
				for (j = 0; j < vendor_tlv->m_nr; j++) {
					printf("%02x ", vendor_tlv->m[j]);
				}
				printf("\n");
				mq = mq_open("/map_efd", O_WRONLY);
				if (mq != -1) {
					mq_send(mq, (const char *)vendor_tlv->m, vendor_tlv->m_nr, 0);
					mq_close(mq);
				}
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
	if (device_role == MAP_ROLE_AGENT)
	{
		register_vendor_message_receive_callback_function(agent_vendor_tlv_handler);
	}
	else
	{
		register_vendor_message_receive_callback_function(controller_vendor_tlv_handler);
	}
}

void vendor_register_append_tlv_call_back_functions(uint8_t device_role)
{
	(void)device_role;
}
