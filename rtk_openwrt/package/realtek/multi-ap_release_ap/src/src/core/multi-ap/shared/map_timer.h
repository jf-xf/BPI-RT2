/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_TIMER_H_
#define _MAP_TIMER_H_

#include <mqueue.h>      // mq_*() functions
#include <stdint.h>

#define MAP_MAX_TIMER_TOKEN (1000)
#define MAP_QUEUE_EVENT_TIMEOUT_PERIODIC (0x01)
#define MAP_QUEUE_EVENT_API_COMMAND (0x02)
#define MAP_QUEUE_EVENT_ONE_SECOND_TIMER (0x03)

struct mapEventTimeOut
{
    uint32_t    timeout_ms;
    uint32_t    token;
};

uint8_t MapRegisterQueueEvent(mqd_t mqdes, uint8_t event_type, void *data);

#endif