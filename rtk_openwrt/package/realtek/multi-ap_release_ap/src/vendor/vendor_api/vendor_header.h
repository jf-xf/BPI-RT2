#ifndef _VENDOR_HEADER_H_
#define _VENDOR_HEADER_H_

#define VENDOR_CUSTOM_TLV_TYPE_1 10
#define VENDOR_CUSTOM_TLV_TYPE_2 11

#define VENDOR_FIFO_1 "/tmp/vendor_fifo_1"
#define VENDOR_FIFO_2 "/tmp/vendor_fifo_2"

uint8_t vendor_oui[3] = { 0x00, 0x11, 0x22 };

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

void insert_length(uint16_t length, uint8_t **ptr)
{
	**ptr = 0xF0 & length;
	(*ptr)++;
	**ptr = 0x0F & length;
	(*ptr)++;
}

#endif