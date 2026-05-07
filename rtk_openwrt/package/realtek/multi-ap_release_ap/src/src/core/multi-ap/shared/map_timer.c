/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */


#include "map_timer.h"
#include <signal.h>
#include <errno.h> // errno
#include <poll.h>  // poll()

#include <pthread.h> // threads and mutex functions
#include <limits.h>  // threads and mutex functions
#include <stdlib.h>  // free(), malloc(), ...
#include <string.h>  // memcpy(), memcmp(), ...
#include <stdio.h>
#include <sys/inotify.h> // inotify_*()
#include <unistd.h>      // read(), sleep()

#include "map_logger.h"

#define EASYMESH_API_COMMAND_FILE "/tmp/map_command"

//#define MAP_MAX_QUEUE_IDS 5 // Number of values that fit in an uint8_t
//static mqd_t           map_queues_id[MAP_MAX_QUEUE_IDS] = { [0 ... MAP_MAX_QUEUE_IDS - 1] = (mqd_t)-1 };

struct _mapTimerHandlerThreadData {
	mqd_t    mqdes;
	uint32_t token;
	uint8_t  periodic;
	timer_t  timer_id;
	uint8_t  map_queue_event;
};

struct _easymeshAPIThreadData {
	mqd_t    mqdes;
};

uint8_t mapSendMessageToMq(mqd_t mqdes, uint8_t *message, uint16_t message_len)
{
	struct mq_attr attr;
	//mqd_t mqdes;
	//mqdes = map_queues_id[queue_id];
	if ((mqd_t)-1 == mqdes) {
		log_error("[ERROR] Invalid queue ID\n");
		return 0;
	}

	if (NULL == message) {
		log_error("[ERROR] Invalid message\n");
		return 0;
	}

	mq_getattr(mqdes, &attr);
	if(attr.mq_curmsgs >= 60) {
		return 1;
	}

	if (0 != mq_send(mqdes, (const char *)message, message_len, 0)) {
		log_error("[ERROR] mq_send returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	return 1;
}

static void _mapTimerHandler(union sigval s)
{
	struct _mapTimerHandlerThreadData *aux;

	uint8_t  message[3 + 4];
	uint16_t packet_len;
	uint8_t  packet_len_msb;
	uint8_t  packet_len_lsb;
	uint8_t  token_msb;
	uint8_t  token_2nd_msb;
	uint8_t  token_3rd_msb;
	uint8_t  token_lsb;

	aux = (struct _mapTimerHandlerThreadData *)s.sival_ptr;

	// In order to build the message that will be inserted into the queue, we
	// need to follow the "message format" defines in the documentation of
	// function 'PLATFORM_REGISTER_QUEUE_EVENT()'
	//
	packet_len = 4;

#if _HOST_IS_LITTLE_ENDIAN_ == 1
	packet_len_msb = *(((uint8_t *)&packet_len) + 1);
	packet_len_lsb = *(((uint8_t *)&packet_len) + 0);

	token_msb     = *(((uint8_t *)&aux->token) + 3);
	token_2nd_msb = *(((uint8_t *)&aux->token) + 2);
	token_3rd_msb = *(((uint8_t *)&aux->token) + 1);
	token_lsb     = *(((uint8_t *)&aux->token) + 0);
#else
	packet_len_msb = *(((uint8_t *)&packet_len) + 0);
	packet_len_lsb = *(((uint8_t *)&packet_len) + 1);

	token_msb     = *(((uint8_t *)&aux->token) + 0);
	token_2nd_msb = *(((uint8_t *)&aux->token) + 1);
	token_3rd_msb = *(((uint8_t *)&aux->token) + 2);
	token_lsb     = *(((uint8_t *)&aux->token) + 3);
#endif

	message[0] = aux->map_queue_event;
	message[1] = packet_len_msb;
	message[2] = packet_len_lsb;
	message[3] = token_msb;
	message[4] = token_2nd_msb;
	message[5] = token_3rd_msb;
	message[6] = token_lsb;

	//log_error("[MAP_API] *Timer handler* Sending %d bytes to queue (%02x, %02x, %02x, ...)\n", 3 + packet_len, message[0], message[1], message[2]);

	if (0 == mapSendMessageToMq(aux->mqdes, message, 3 + packet_len)) {
		log_error("[ERROR] *Timer handler* Error sending message to queue from _timerHandler()\n");
	}

	if (1 == aux->periodic) {
		// Periodic timer are automatically re-armed. We don't need to do
		// anything
	} else {
		// Delete the asociater timer
		//
		timer_delete(aux->timer_id);

		// Free 'struct _timerHandlerThreadData', as we don't need it any more
		//
		free(aux);
	}

	return;
}

static void *_easymeshAPIThread(void *p)
{
	FILE *fd_tmp;
	int   fdraw_tmp;

	struct pollfd fdset[2];
	mqd_t    mqdes;

	mqdes = ((struct _easymeshAPIThreadData *)p)->mqdes;
	fd_tmp = fopen(EASYMESH_API_COMMAND_FILE, "w+e");
	if (NULL == fd_tmp) {
		printf("*API thread* Could not create tmp file %s\n", EASYMESH_API_COMMAND_FILE);
		free(p);
		return NULL;
	}
	fclose(fd_tmp);

	// ...and then add a "watch" that triggers when its content changes (ie.
	// when someone does a "echo" of the file or writes to it, for example).
	//
	if (-1 == (fdraw_tmp = inotify_init1(IN_CLOEXEC))) {
		printf("*API thread* inotify_init() returned with errno=%d (%s)\n", errno, strerror(errno));

		free(p);
		return NULL;
	}
	if (-1 == inotify_add_watch(fdraw_tmp, EASYMESH_API_COMMAND_FILE, IN_CLOSE_WRITE )) {
		printf("*API thread* inotify_add_watch() returned with errno=%d (%s)\n", errno, strerror(errno));

		free(p);
		return NULL;
	}
	// At this point we have file descriptor (fdraw_tmp)
	// that we can monitor with a call to "poll()"
	//
	while (1) {
		int   nfds;

		memset((void *)fdset, 0, sizeof(fdset));

		fdset[0].fd     = fdraw_tmp;
		fdset[0].events = POLLIN;
		nfds            = 1;

		// The thread will block here (forever, timeout = -1), until there is
		// a change in the file descriptor ("changes" in the "tmp"
		// file fd are cause by "modify" changes -such as the content change).

		if (0 > poll(fdset, nfds, -1)) {
			printf("*API thread* poll() returned with errno=%d (%s)\n", errno, strerror(errno));
			break;
		}
		if (fdset[0].revents & POLLIN) {
			struct inotify_event event;
			printf("*API thread* content has been changed!\n");
			// We must "read()" from the "tmp" fd to "consume" the event, or
			// else the next call to "poll() won't block.
			//
			if (-1 == read(fdraw_tmp, &event, sizeof(event))) {
				log_error("[ERROR] read() returned with errno=%d (%s)\n", errno, strerror(errno));
				continue;
			}
		}

		FILE *fd_tmp = fopen(EASYMESH_API_COMMAND_FILE, "re");
		if (NULL == fd_tmp) {
			printf("Could not open tmp file %s\n", EASYMESH_API_COMMAND_FILE);
			continue;
		}
		unsigned char command[4];
		fgets((char *)command,4,fd_tmp);
		printf("read API command: %s\n",command);
        fclose(fd_tmp);

		unsigned char message[5];
		message[0] = MAP_QUEUE_EVENT_API_COMMAND;
		message[1] = 0x00;
		message[2] = 0x02;
		message[3] = command[0];
		message[4] = command[2];

		printf("*API thread* Sending 5 bytes to queue (0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x)\n",
			message[0], message[1], message[2], message[3], message[4]);

		if (0 == mapSendMessageToMq(mqdes, message, 5)) {
			printf("*API thread* Error sending message to queue from _easymeshAPIThread()\n");
		}
	}

	printf("*API thread* Exiting...\n");

	free(p);
	return NULL;
}

uint8_t MapRegisterQueueEvent(mqd_t mqdes, uint8_t event_type, void *data)
{
	switch (event_type) {

	case MAP_QUEUE_EVENT_TIMEOUT_PERIODIC:
	case MAP_QUEUE_EVENT_ONE_SECOND_TIMER:
	{
		struct mapEventTimeOut *           p1;
		struct _mapTimerHandlerThreadData *p2;

		struct sigevent   se;
		struct itimerspec its;
		timer_t           timer_id;

		p1 = (struct mapEventTimeOut *)data;

		if (p1->token > MAP_MAX_TIMER_TOKEN) {
			// Invalid arguments
			//
			return 0;
		}

		p2 = (struct _mapTimerHandlerThreadData *)malloc(sizeof(struct _mapTimerHandlerThreadData));
		if (NULL == p2) {
			// Out of memory
			//
			return 0;
		}

		p2->mqdes    = mqdes;
		p2->token    = p1->token;
		p2->periodic = 1;
		p2->map_queue_event = event_type;

		// Next, create the timer. Note that it will be automatically
		// destroyed (by us) in the callback function
		//
		memset(&se, 0, sizeof(se));
		se.sigev_notify          = SIGEV_THREAD;
		se.sigev_notify_function = _mapTimerHandler;
		se.sigev_value.sival_ptr = (void *)p2;

		if (-1 == timer_create(CLOCK_REALTIME, &se, &timer_id)) {
			// Failed to create a new timer
			//
			log_error("[ERROR] timer_create() returned with errno=%d (%s)\n", errno, strerror(errno));
			free(p2);
			return 0;
		}
		p2->timer_id = timer_id;

		// Finally, arm/start the timer
		//
		its.it_value.tv_sec     = p1->timeout_ms / 1000;
		its.it_value.tv_nsec    = (p1->timeout_ms % 1000) * 1000000;
		its.it_interval.tv_sec  = its.it_value.tv_sec;
		its.it_interval.tv_nsec = its.it_value.tv_nsec;

		if (0 != timer_settime(timer_id, 0, &its, NULL)) {
			// Problems arming the timer
			//
			free(p2);
			timer_delete(timer_id);
			return 0;
		}

		break;
	}
	case MAP_QUEUE_EVENT_API_COMMAND: {
		pthread_t                     thread;

		struct _easymeshAPIThreadData *p;

		p = (struct _easymeshAPIThreadData *)malloc(sizeof(struct _easymeshAPIThreadData));
		if (NULL == p) {
			// Out of memory
			//
			return 0;
		}

		p->mqdes = mqdes;
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
		pthread_create(&thread, &attr, _easymeshAPIThread, (void *)p);
		pthread_detach(thread);
		break;

	}
	default: {
		// Unknown event type!!
		//
		return 0;
	}
	}

	return 1;
}
