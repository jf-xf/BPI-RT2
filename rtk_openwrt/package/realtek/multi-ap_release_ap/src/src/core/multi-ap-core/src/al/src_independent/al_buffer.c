/*
 *  Mulit-AP Message Buffer for Wi-Fi Test Suite
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include "platform.h"

#include "al_buffer.h"

#include <unistd.h>
#include <stdio.h>

#define BUFFER_INDICATOR "/tmp/map_msgbuf_on"
#define BUFFER_OUTPUT_FILE "/tmp/map_msgbuf"

#define BUFFER_FAIL -1
#define BUFFER_OK 0

//////////////////////////////////////////
//         Internal Variables           //
//////////////////////////////////////////
static INT32U msg_counter = 1;

//////////////////////////////////////////
//         Internal Functions           //
//////////////////////////////////////////
static int match_filter(INT16U msg_type)
{
	FILE * fp;
	char * line = NULL;
	size_t line_len;
	char   msg_type_hex[8];
	int    ret = BUFFER_FAIL;

	if ((fp = fopen(BUFFER_INDICATOR, "re")) == NULL)
		return BUFFER_FAIL;

	if (getline(&line, &line_len, fp) == -1) {
		goto fail;
	}

	sprintf(msg_type_hex, "0x%04X", msg_type);
	if (PLATFORM_STRSTR(line, msg_type_hex) == NULL) {
		goto fail;
	}

	PLATFORM_PRINTF_INFO("[BUFFER] msg_type: %s\n", msg_type_hex);
	ret = BUFFER_OK;

fail:
	free(line);
	fclose(fp);
	return ret;
}

static int append_record(char *record)
{
	FILE *fp = NULL;

	if ((fp = fopen(BUFFER_OUTPUT_FILE, "ae")) == NULL) {
		PLATFORM_PRINTF_WARNING("[BUFFER] fopen null\n");
		return BUFFER_FAIL;
	}

	fputs(record, fp);
	fputs("\n", fp);
	fclose(fp);
	return BUFFER_OK;
}

//////////////////////////////////////////
//           Public Functions           //
//////////////////////////////////////////
void buffer_cmdu_message(struct CMDU *message)
{
	char  msgbuf[384];
	INT8U i = 0;

	// Proceed only if CMDU Type is matched
	if (match_filter(message->message_type) != BUFFER_OK)
		return;

	// Forge record
	snprintf(msgbuf, sizeof(msgbuf), "message,0x%04X,MID,0x%04X,TLVList,%u",
	         message->message_type, message->message_id, msg_counter++);

	while (message->list_of_TLVs[i]) {
		char tlv_id_hex[8];
		sprintf(tlv_id_hex, "_0x%02X", message->list_of_TLVs[i][0]);
		PLATFORM_STRNCAT(msgbuf, tlv_id_hex, sizeof(msgbuf) - 1);
		i++;
	}

	// Append to file
	append_record(msgbuf);
}
