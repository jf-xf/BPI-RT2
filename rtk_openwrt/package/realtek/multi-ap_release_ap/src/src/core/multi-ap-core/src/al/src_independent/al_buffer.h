/*
 *  Mulit-AP Message Buffer for Wi-Fi Test Suite
 *
 *  Wi-Fi Test Suite requires CTTController and CTTAgent to support caching
 *  of CMDU packets. Hence, the purpose of this message buffer is to record
 *  the sent/received CMDUs, and save it into a file. Then, the control agent
 *  should read the file for further processing.
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#ifndef _AL_BUFFER_H_
#define _AL_BUFFER_H_

#include "1905_cmdus.h"

/**
 * buffer_cmdu_message - Add the CMDU packet into the message buffer
 *
 * @message: the pointer of the CMDU input
 */
void buffer_cmdu_message(struct CMDU *message);

#endif /* _AL_BUFFER_H_ */