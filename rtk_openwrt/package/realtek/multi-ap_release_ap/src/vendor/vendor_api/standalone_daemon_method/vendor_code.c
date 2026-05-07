#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "map_tlvs.h"
#include "map_utils.h"
#include "map_commands.h"

#include "vendor_header.h"

void send_data(uint8_t *target_al_mac, uint16_t payload_len, uint8_t *payload)
{
	uint8_t relay_indicator = 0;

	int i = 0;
	printf("Message to be send is: \n");
	for (i = 0; i < payload_len; i++) {
		printf("%02x ", payload[i]);
	}
	printf("\n");

	send_vendor_specific_message(target_al_mac, relay_indicator, vendor_oui, payload_len, (uint8_t *)payload);
}

int main(void)
{
	uint8_t  target_al_mac[6] = { 0x00, 0xE2, 0x18, 0x81, 0x97, 0xC1 };
	char *   test_message     = "654321";
	uint16_t test_message_len = strlen(test_message) + 1;

	uint16_t payload_len_1 = test_message_len + 3;

	mkfifo(VENDOR_FIFO_1, 0666);

	uint8_t  payload_1[128] = { 0 };
	uint8_t *packet_ptr     = payload_1;
	*packet_ptr             = VENDOR_CUSTOM_TLV_TYPE_1;
	packet_ptr++;
	insert_length(test_message_len, &packet_ptr);
	memcpy(packet_ptr, test_message, test_message_len);

	send_data(target_al_mac, payload_len_1, payload_1);

	char buffer[1500];
	int  buffer_size;
	int  i = 0;

	int fd = open(VENDOR_FIFO_1, O_RDONLY);

	buffer_size = read(fd, buffer, sizeof(buffer));
	printf("##############################################################################\n");
	printf("Received vendor data with length %d!!!\n", buffer_size);
	for (i = 0; i < buffer_size; i++) {
		printf("%02x ", buffer[i]);
	}
	printf("\n");
	printf("##############################################################################\n");

	remove(VENDOR_FIFO_1);
	close(fd);

	return 0;
}