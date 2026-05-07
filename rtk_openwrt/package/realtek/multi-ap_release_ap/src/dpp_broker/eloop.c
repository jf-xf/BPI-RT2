/*
 * Event loop based on select() loop
 * Copyright (c) 2002-2009, Jouni Malinen <j@w1.fi>
 * 
 * This software may be distributed, used, and modified under the terms of
 * BSD license:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name(s) of the above-listed copyright holder(s) nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <poll.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include "list.h"
#include "eloop.h"

struct eloop_sock {
	int sock;
	void *eloop_data;
	void *user_data;
	eloop_sock_handler handler;
};

struct eloop_timeout {
	struct dl_list list;
	struct os_reltime time;
	void *eloop_data;
	void *user_data;
	eloop_timeout_handler handler;
};

struct eloop_signal {
	int sig;
	void *user_data;
	eloop_signal_handler handler;
	int signaled;
};

struct eloop_sock_table {
	size_t count;
	struct eloop_sock *table;
	eloop_event_type type;
	int changed;
};

struct eloop_data {
	int max_sock;
	size_t count; /* sum of all table counts */
	size_t max_pollfd_map; /* number of pollfds_map currently allocated */
	size_t max_poll_fds; /* number of pollfds currently allocated */
	struct pollfd *pollfds;
	struct pollfd **pollfds_map;
	struct eloop_sock_table readers;
	struct eloop_sock_table writers;
	struct eloop_sock_table exceptions;

	struct dl_list timeout;

	size_t signal_count;
	struct eloop_signal *signals;
	int signaled;
	int pending_terminate;

	int terminate;
};

static struct eloop_data eloop;

static inline void * os_realloc_array(void *ptr, size_t nmemb, size_t size)
{
	if (size && nmemb > (~(size_t) 0) / size)
		return NULL;
	return realloc(ptr, nmemb * size);
}

static inline int os_reltime_before(struct os_reltime *a,
				    struct os_reltime *b)
{
	return (a->sec < b->sec) ||
	       (a->sec == b->sec && a->usec < b->usec);
}

static inline void os_reltime_sub(struct os_reltime *a, struct os_reltime *b,
				  struct os_reltime *res)
{
	res->sec = a->sec - b->sec;
	res->usec = a->usec - b->usec;
	if (res->usec < 0) {
		res->sec--;
		res->usec += 1000000;
	}
}

int os_get_reltime(struct os_reltime *t)
{
	int res;
	struct timeval tv;
	res = gettimeofday(&tv, NULL);
	t->sec = tv.tv_sec;
	t->usec = tv.tv_usec;
	return res;
}

int eloop_init(void)
{
	memset(&eloop, 0, sizeof(eloop));
	dl_list_init(&eloop.timeout);
	return 0;
}

static int eloop_sock_table_add_sock(struct eloop_sock_table *table,
                                     int sock, eloop_sock_handler handler,
                                     void *eloop_data, void *user_data)
{
	struct eloop_sock *tmp;
	int new_max_sock;

	if (sock > eloop.max_sock)
		new_max_sock = sock;
	else
		new_max_sock = eloop.max_sock;

	if (table == NULL)
		return -1;

	if ((size_t) new_max_sock >= eloop.max_pollfd_map) {
		struct pollfd **nmap;
		nmap = os_realloc_array(eloop.pollfds_map, new_max_sock + 50,
					sizeof(struct pollfd *));
		if (nmap == NULL)
			return -1;

		eloop.max_pollfd_map = new_max_sock + 50;
		eloop.pollfds_map = nmap;
	}

	if (eloop.count + 1 > eloop.max_poll_fds) {
		struct pollfd *n;
		size_t nmax = eloop.count + 1 + 50;

		n = os_realloc_array(eloop.pollfds, nmax,
				     sizeof(struct pollfd));
		if (n == NULL)
			return -1;

		eloop.max_poll_fds = nmax;
		eloop.pollfds = n;
	}

	tmp = os_realloc_array(table->table, table->count + 1,
			       sizeof(struct eloop_sock));
	if (tmp == NULL) {
		return -1;
	}

	tmp[table->count].sock = sock;
	tmp[table->count].eloop_data = eloop_data;
	tmp[table->count].user_data = user_data;
	tmp[table->count].handler = handler;
	table->count++;
	table->table = tmp;
	eloop.max_sock = new_max_sock;
	eloop.count++;
	table->changed = 1;

	return 0;
}


static void eloop_sock_table_remove_sock(struct eloop_sock_table *table,
                                         int sock)
{
	size_t i;

	if (table == NULL || table->table == NULL || table->count == 0)
		return;

	for (i = 0; i < table->count; i++) {
		if (table->table[i].sock == sock)
			break;
	}
	if (i == table->count)
		return;

	if (i != table->count - 1) {
		memmove(&table->table[i], &table->table[i + 1],
			   (table->count - i - 1) *
			   sizeof(struct eloop_sock));
	}
	table->count--;
	eloop.count--;
	table->changed = 1;
}


static struct pollfd * find_pollfd(struct pollfd **pollfds_map, int fd, int mx)
{
	if (fd < mx && fd >= 0)
		return pollfds_map[fd];
	return NULL;
}


static int eloop_sock_table_set_fds(struct eloop_sock_table *readers,
				    struct eloop_sock_table *writers,
				    struct eloop_sock_table *exceptions,
				    struct pollfd *pollfds,
				    struct pollfd **pollfds_map,
				    int max_pollfd_map)
{
	size_t i;
	int nxt = 0;
	int fd;
	struct pollfd *pfd;

	/* Clear pollfd lookup map. It will be re-populated below. */
	memset(pollfds_map, 0, sizeof(struct pollfd *) * max_pollfd_map);

	if (readers && readers->table) {
		for (i = 0; i < readers->count; i++) {
			fd = readers->table[i].sock;
			assert(fd >= 0 && fd < max_pollfd_map);
			pollfds[nxt].fd = fd;
			pollfds[nxt].events = POLLIN;
			pollfds[nxt].revents = 0;
			pollfds_map[fd] = &(pollfds[nxt]);
			nxt++;
		}
	}

	if (writers && writers->table) {
		for (i = 0; i < writers->count; i++) {
			/*
			 * See if we already added this descriptor, update it
			 * if so.
			 */
			fd = writers->table[i].sock;
			assert(fd >= 0 && fd < max_pollfd_map);
			pfd = pollfds_map[fd];
			if (!pfd) {
				pfd = &(pollfds[nxt]);
				pfd->events = 0;
				pfd->fd = fd;
				pollfds[i].revents = 0;
				pollfds_map[fd] = pfd;
				nxt++;
			}
			pfd->events |= POLLOUT;
		}
	}

	/*
	 * Exceptions are always checked when using poll, but I suppose it's
	 * possible that someone registered a socket *only* for exception
	 * handling. Set the POLLIN bit in this case.
	 */
	if (exceptions && exceptions->table) {
		for (i = 0; i < exceptions->count; i++) {
			/*
			 * See if we already added this descriptor, just use it
			 * if so.
			 */
			fd = exceptions->table[i].sock;
			assert(fd >= 0 && fd < max_pollfd_map);
			pfd = pollfds_map[fd];
			if (!pfd) {
				pfd = &(pollfds[nxt]);
				pfd->events = POLLIN;
				pfd->fd = fd;
				pollfds[i].revents = 0;
				pollfds_map[fd] = pfd;
				nxt++;
			}
		}
	}

	return nxt;
}


static int eloop_sock_table_dispatch_table(struct eloop_sock_table *table,
					   struct pollfd **pollfds_map,
					   int max_pollfd_map,
					   short int revents)
{
	size_t i;
	struct pollfd *pfd;

	if (!table || !table->table)
		return 0;

	table->changed = 0;
	for (i = 0; i < table->count; i++) {
		pfd = find_pollfd(pollfds_map, table->table[i].sock,
				  max_pollfd_map);
		if (!pfd)
			continue;

		if (!(pfd->revents & revents))
			continue;

		table->table[i].handler(table->table[i].sock,
					table->table[i].eloop_data,
					table->table[i].user_data);
		if (table->changed)
			return 1;
	}

	return 0;
}


static void eloop_sock_table_dispatch(struct eloop_sock_table *readers,
				      struct eloop_sock_table *writers,
				      struct eloop_sock_table *exceptions,
				      struct pollfd **pollfds_map,
				      int max_pollfd_map)
{
	if (eloop_sock_table_dispatch_table(readers, pollfds_map,
					    max_pollfd_map, POLLIN | POLLERR |
					    POLLHUP))
		return; /* pollfds may be invalid at this point */

	if (eloop_sock_table_dispatch_table(writers, pollfds_map,
					    max_pollfd_map, POLLOUT))
		return; /* pollfds may be invalid at this point */

	eloop_sock_table_dispatch_table(exceptions, pollfds_map,
					max_pollfd_map, POLLERR | POLLHUP);
}


static void eloop_sock_table_destroy(struct eloop_sock_table *table)
{
	if (table) {
		size_t i;

		for (i = 0; i < table->count && table->table; i++) {
			printf("ELOOP: remaining socket: "
				   "sock=%d eloop_data=%p user_data=%p "
				   "handler=%p\n",
				   table->table[i].sock,
				   table->table[i].eloop_data,
				   table->table[i].user_data,
				   table->table[i].handler);
		}
		free(table->table);
	}
}


int eloop_register_read_sock(int sock, eloop_sock_handler handler,
			     void *eloop_data, void *user_data)
{
	return eloop_register_sock(sock, EVENT_TYPE_READ, handler,
				   eloop_data, user_data);
}


void eloop_unregister_read_sock(int sock)
{
	eloop_unregister_sock(sock, EVENT_TYPE_READ);
}


static struct eloop_sock_table *eloop_get_sock_table(eloop_event_type type)
{
	switch (type) {
	case EVENT_TYPE_READ:
		return &eloop.readers;
	case EVENT_TYPE_WRITE:
		return &eloop.writers;
	case EVENT_TYPE_EXCEPTION:
		return &eloop.exceptions;
	}

	return NULL;
}


int eloop_register_sock(int sock, eloop_event_type type,
			eloop_sock_handler handler,
			void *eloop_data, void *user_data)
{
	struct eloop_sock_table *table;

	assert(sock >= 0);
	table = eloop_get_sock_table(type);
	return eloop_sock_table_add_sock(table, sock, handler,
					 eloop_data, user_data);
}


void eloop_unregister_sock(int sock, eloop_event_type type)
{
	struct eloop_sock_table *table;

	table = eloop_get_sock_table(type);
	eloop_sock_table_remove_sock(table, sock);
}


int eloop_register_timeout(unsigned int secs, unsigned int usecs,
			   eloop_timeout_handler handler,
			   void *eloop_data, void *user_data)
{
	struct eloop_timeout *timeout, *tmp;
	long now_sec;

	timeout = malloc(sizeof(*timeout));
	memset(timeout, 0, sizeof(*timeout));
	if (timeout == NULL)
		return -1;
	if (os_get_reltime(&timeout->time) < 0) {
		free(timeout);
		return -1;
	}
	now_sec = timeout->time.sec;
	timeout->time.sec += secs;
	if (timeout->time.sec < now_sec)
		goto overflow;
	timeout->time.usec += usecs;
	while (timeout->time.usec >= 1000000) {
		timeout->time.sec++;
		timeout->time.usec -= 1000000;
	}
	if (timeout->time.sec < now_sec)
		goto overflow;
	timeout->eloop_data = eloop_data;
	timeout->user_data = user_data;
	timeout->handler = handler;

	/* Maintain timeouts in order of increasing time */
	dl_list_for_each(tmp, &eloop.timeout, struct eloop_timeout, list) {
		if (os_reltime_before(&timeout->time, &tmp->time)) {
			dl_list_add(tmp->list.prev, &timeout->list);
			return 0;
		}
	}
	dl_list_add_tail(&eloop.timeout, &timeout->list);

	return 0;

overflow:
	/*
	 * Integer overflow - assume long enough timeout to be assumed
	 * to be infinite, i.e., the timeout would never happen.
	 */
	printf("ELOOP: Too long timeout (secs=%u usecs=%u) to ever happen - ignore it\n",
		   secs,usecs);
	free(timeout);
	return 0;
}


static void eloop_remove_timeout(struct eloop_timeout *timeout)
{
	dl_list_del(&timeout->list);
	free(timeout);
}


int eloop_cancel_timeout(eloop_timeout_handler handler,
			 void *eloop_data, void *user_data)
{
	struct eloop_timeout *timeout, *prev;
	int removed = 0;

	dl_list_for_each_safe(timeout, prev, &eloop.timeout,
			      struct eloop_timeout, list) {
		if (timeout->handler == handler &&
		    (timeout->eloop_data == eloop_data ||
		     eloop_data == ELOOP_ALL_CTX) &&
		    (timeout->user_data == user_data ||
		     user_data == ELOOP_ALL_CTX)) {
			eloop_remove_timeout(timeout);
			removed++;
		}
	}

	return removed;
}


int eloop_cancel_timeout_one(eloop_timeout_handler handler,
			     void *eloop_data, void *user_data,
			     struct os_reltime *remaining)
{
	struct eloop_timeout *timeout, *prev;
	int removed = 0;
	struct os_reltime now;

	os_get_reltime(&now);
	remaining->sec = remaining->usec = 0;

	dl_list_for_each_safe(timeout, prev, &eloop.timeout,
			      struct eloop_timeout, list) {
		if (timeout->handler == handler &&
		    (timeout->eloop_data == eloop_data) &&
		    (timeout->user_data == user_data)) {
			removed = 1;
			if (os_reltime_before(&now, &timeout->time))
				os_reltime_sub(&timeout->time, &now, remaining);
			eloop_remove_timeout(timeout);
			break;
		}
	}
	return removed;
}


int eloop_is_timeout_registered(eloop_timeout_handler handler,
				void *eloop_data, void *user_data)
{
	struct eloop_timeout *tmp;

	dl_list_for_each(tmp, &eloop.timeout, struct eloop_timeout, list) {
		if (tmp->handler == handler &&
		    tmp->eloop_data == eloop_data &&
		    tmp->user_data == user_data)
			return 1;
	}

	return 0;
}


int eloop_deplete_timeout(unsigned int req_secs, unsigned int req_usecs,
			  eloop_timeout_handler handler, void *eloop_data,
			  void *user_data)
{
	struct os_reltime now, requested, remaining;
	struct eloop_timeout *tmp;

	dl_list_for_each(tmp, &eloop.timeout, struct eloop_timeout, list) {
		if (tmp->handler == handler &&
		    tmp->eloop_data == eloop_data &&
		    tmp->user_data == user_data) {
			requested.sec = req_secs;
			requested.usec = req_usecs;
			os_get_reltime(&now);
			os_reltime_sub(&tmp->time, &now, &remaining);
			if (os_reltime_before(&requested, &remaining)) {
				eloop_cancel_timeout(handler, eloop_data,
						     user_data);
				eloop_register_timeout(requested.sec,
						       requested.usec,
						       handler, eloop_data,
						       user_data);
				return 1;
			}
			return 0;
		}
	}

	return -1;
}


int eloop_replenish_timeout(unsigned int req_secs, unsigned int req_usecs,
			    eloop_timeout_handler handler, void *eloop_data,
			    void *user_data)
{
	struct os_reltime now, requested, remaining;
	struct eloop_timeout *tmp;

	dl_list_for_each(tmp, &eloop.timeout, struct eloop_timeout, list) {
		if (tmp->handler == handler &&
		    tmp->eloop_data == eloop_data &&
		    tmp->user_data == user_data) {
			requested.sec = req_secs;
			requested.usec = req_usecs;
			os_get_reltime(&now);
			os_reltime_sub(&tmp->time, &now, &remaining);
			if (os_reltime_before(&remaining, &requested)) {
				eloop_cancel_timeout(handler, eloop_data,
						     user_data);
				eloop_register_timeout(requested.sec,
						       requested.usec,
						       handler, eloop_data,
						       user_data);
				return 1;
			}
			return 0;
		}
	}

	return -1;
}


static void eloop_handle_alarm(int sig)
{
	printf("eloop: could not process SIGINT or SIGTERM in "
		   "two seconds. Looks like there\n"
		   "is a bug that ends up in a busy loop that "
		   "prevents clean shutdown.\n"
		   "Killing program forcefully.\n");
	exit(1);
}


static void eloop_handle_signal(int sig)
{
	size_t i;

	if ((sig == SIGINT || sig == SIGTERM) && !eloop.pending_terminate) {
		/* Use SIGALRM to break out from potential busy loops that
		 * would not allow the program to be killed. */
		eloop.pending_terminate = 1;
		signal(SIGALRM, eloop_handle_alarm);
		alarm(2);
	}

	eloop.signaled++;
	for (i = 0; i < eloop.signal_count; i++) {
		if (eloop.signals[i].sig == sig) {
			eloop.signals[i].signaled++;
			break;
		}
	}
}


static void eloop_process_pending_signals(void)
{
	size_t i;

	if (eloop.signaled == 0)
		return;
	eloop.signaled = 0;

	if (eloop.pending_terminate) {
		alarm(0);
		eloop.pending_terminate = 0;
	}

	for (i = 0; i < eloop.signal_count; i++) {
		if (eloop.signals[i].signaled) {
			eloop.signals[i].signaled = 0;
			eloop.signals[i].handler(eloop.signals[i].sig,
						 eloop.signals[i].user_data);
		}
	}
}


int eloop_register_signal(int sig, eloop_signal_handler handler,
			  void *user_data)
{
	struct eloop_signal *tmp;

	tmp = os_realloc_array(eloop.signals, eloop.signal_count + 1,
			       sizeof(struct eloop_signal));
	if (tmp == NULL)
		return -1;

	tmp[eloop.signal_count].sig = sig;
	tmp[eloop.signal_count].user_data = user_data;
	tmp[eloop.signal_count].handler = handler;
	tmp[eloop.signal_count].signaled = 0;
	eloop.signal_count++;
	eloop.signals = tmp;
	signal(sig, eloop_handle_signal);

	return 0;
}


int eloop_register_signal_terminate(eloop_signal_handler handler,
				    void *user_data)
{
	int ret = eloop_register_signal(SIGINT, handler, user_data);
	if (ret == 0)
		ret = eloop_register_signal(SIGTERM, handler, user_data);
	return ret;
}


int eloop_register_signal_reconfig(eloop_signal_handler handler,
				   void *user_data)
{
	return eloop_register_signal(SIGHUP, handler, user_data);
}


void eloop_run(void)
{
	int num_poll_fds;
	int timeout_ms = 0;
	int res;
	struct os_reltime tv, now;

	while (!eloop.terminate &&
	       (!dl_list_empty(&eloop.timeout) || eloop.readers.count > 0 ||
		eloop.writers.count > 0 || eloop.exceptions.count > 0)) {
		struct eloop_timeout *timeout;
		if (eloop.pending_terminate) {
			/*
			 * This may happen in some corner cases where a signal
			 * is received during a blocking operation. We need to
			 * process the pending signals and exit if requested to
			 * avoid hitting the SIGALRM limit if the blocking
			 * operation took more than two seconds.
			 */
			eloop_process_pending_signals();
			if (eloop.terminate)
				break;
		}

		timeout = dl_list_first(&eloop.timeout, struct eloop_timeout,
					list);
		if (timeout) {
			os_get_reltime(&now);
			if (os_reltime_before(&now, &timeout->time))
				os_reltime_sub(&timeout->time, &now, &tv);
			else
				tv.sec = tv.usec = 0;
			timeout_ms = tv.sec * 1000 + tv.usec / 1000;
		}

		num_poll_fds = eloop_sock_table_set_fds(
			&eloop.readers, &eloop.writers, &eloop.exceptions,
			eloop.pollfds, eloop.pollfds_map,
			eloop.max_pollfd_map);
		res = poll(eloop.pollfds, num_poll_fds,
			   timeout ? timeout_ms : -1);

		if (res < 0 && errno != EINTR && errno != 0) {
			printf("eloop: poll: %s\n", strerror(errno));
			goto out;
		}

		eloop.readers.changed = 0;
		eloop.writers.changed = 0;
		eloop.exceptions.changed = 0;

		eloop_process_pending_signals();

		/* check if some registered timeouts have occurred */
		timeout = dl_list_first(&eloop.timeout, struct eloop_timeout,
					list);
		if (timeout) {
			os_get_reltime(&now);
			if (!os_reltime_before(&now, &timeout->time)) {
				void *eloop_data = timeout->eloop_data;
				void *user_data = timeout->user_data;
				eloop_timeout_handler handler =
					timeout->handler;
				eloop_remove_timeout(timeout);
				handler(eloop_data, user_data);
			}

		}

		if (res <= 0)
			continue;

		if (eloop.readers.changed ||
		    eloop.writers.changed ||
		    eloop.exceptions.changed) {
			 /*
			  * Sockets may have been closed and reopened with the
			  * same FD in the signal or timeout handlers, so we
			  * must skip the previous results and check again
			  * whether any of the currently registered sockets have
			  * events.
			  */
			continue;
		}

		eloop_sock_table_dispatch(&eloop.readers, &eloop.writers,
					  &eloop.exceptions, eloop.pollfds_map,
					  eloop.max_pollfd_map);
	}

	eloop.terminate = 0;
out:
	return;
}


void eloop_terminate(void)
{
	eloop.terminate = 1;
}


void eloop_destroy(void)
{
	struct eloop_timeout *timeout, *prev;
	struct os_reltime now;

	os_get_reltime(&now);
	dl_list_for_each_safe(timeout, prev, &eloop.timeout,
			      struct eloop_timeout, list) {
		int sec, usec;
		sec = timeout->time.sec - now.sec;
		usec = timeout->time.usec - now.usec;
		if (timeout->time.usec < now.usec) {
			sec--;
			usec += 1000000;
		}
		printf("ELOOP: remaining timeout: %d.%06d "
			   "eloop_data=%p user_data=%p handler=%p\n",
			   sec, usec, timeout->eloop_data, timeout->user_data,
			   timeout->handler);
		eloop_remove_timeout(timeout);
	}
	eloop_sock_table_destroy(&eloop.readers);
	eloop_sock_table_destroy(&eloop.writers);
	eloop_sock_table_destroy(&eloop.exceptions);
	free(eloop.signals);

	free(eloop.pollfds);
	free(eloop.pollfds_map);
}


int eloop_terminated(void)
{
	return eloop.terminate || eloop.pending_terminate;
}


void eloop_wait_for_read_sock(int sock)
{
	struct pollfd pfd;
	int res;

	if (sock < 0)
		return;

	memset(&pfd, 0, sizeof(pfd));
	pfd.fd = sock;
	pfd.events = POLLIN;

	res = poll(&pfd, 1, -1);
	if (res < 0) {
		printf("poll returns %d \n", res);
	}
}
