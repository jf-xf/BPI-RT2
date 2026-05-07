#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include "timerEventList.h"

/****************************************
* tlist_header_init
****************************************/
void tlist_header_init(tList_s *list)
{
	if (NULL == list)
		return;

	list->headFlag = TRUE;			
	list->prev = list->next = list;
}
/****************************************
* tlist_node_init
****************************************/
void tlist_node_init(tList_s *list)
{
	if (NULL == list)
		return;

	list->headFlag = FALSE;			
	list->prev = list->next = list;
}
/****************************************
* tlist_size
****************************************/
int tlist_size(tList_s *headList)
{
	tList_s *list;
	int listCnt;

	if (NULL == headList)
		return 0;

	listCnt = 0;
	for (list = tlist_next(headList); list != NULL; list = tlist_next(list))
		listCnt++;
	
	return listCnt;
}
/****************************************
* tlist_get
****************************************/
tList_s *tlist_get(tList_s *headList, int index)
{
	tList_s *list;
	int n;

	if (NULL == headList)
		return NULL;

	list = tlist_next(headList);
	for (n=0; n<index; n++) {
		if (NULL == list)
			break;
		list = tlist_next(list);
	}
	
	return list;
}
/****************************************
* tlist_getlast
****************************************/
tList_s *tlist_getlast(tList_s *headList)
{
	tList_s *list, *list2;

	if (NULL == headList)
		return NULL;

	for (list = tlist_next(headList); list != NULL; ){
		list2 = tlist_next(list);
		if(list2 == NULL) break;
		list = list2;
	}
	return list;
}
/****************************************
* tlist_insert
****************************************/
void tlist_insert( tList_s *prevList, tList_s *list)
{
	if ((NULL == prevList) || (NULL == list))
		return;

	list->prev = prevList;
	list->next = prevList->next;
	prevList->next->prev = list;
	prevList->next = list;
}

/****************************************
* tlist_add
****************************************/
void tlist_add(tList_s *headList, tList_s *list)
{
	if ((NULL == headList) || (NULL == list))
		return;

	if (NULL == headList->prev)
		return;
	
	tlist_insert(headList->prev, list);
}

/****************************************
* tlist_remove
****************************************/
void tlist_remove(tList_s *list)
{
	if (NULL == list)
		return;

	if ((NULL == list->prev) || (NULL == list->next))
		return;
	
	list->prev->next = list->next;
	list->next->prev = list->prev;
	list->prev = list->next = list;
}
/****************************************
* tlist_prev
****************************************/
tList_s *tlist_prev(tList_s *list)
{
	if (NULL == list)
		return NULL;

	if (NULL == list->prev)
		return NULL;
	
	if (list->prev->headFlag == TRUE)
		return NULL;
	
	if (list->prev == list)
		return NULL;
		
	return list->prev;
}
/****************************************
* tlist_next
****************************************/
tList_s *tlist_next(tList_s *list)
{
	if (NULL == list)
		return NULL;

	if (NULL == list->next)
		return NULL;
	
	if (list->next->headFlag == TRUE)
		return NULL;
	
	if (list->next == list)
		return NULL;

	return list->next;
}
/****************************************
* tlist_clear
****************************************/
void tlist_clear(tList_s *headList, TLIST_DESTFUNC destructorFunc)
{
	tList_s *list;
	
	if (NULL == headList)
		return;

	list = tlist_next(headList);
	while(list != NULL) {
		tlist_remove(list);
		//Theo Beisch: use destructorFunc or just free(listElement)
		if (destructorFunc != NULL){
			destructorFunc((void*)list);
		} else {
			free(list);
		}
		list = tlist_next(headList);
	}
}


/****************************************
* Timer Function
****************************************/
static tEventLists timerEventLists;
static char do_runTimer = 0;

void init_timer_event(void)
{
	memset((void*)&timerEventLists, 0, sizeof(tEventLists));
	tlist_header_init((tList_s *)&timerEventLists);
	do_runTimer = 1;
}

int get_timer_event_size(void)
{
	return tlist_size((tList_s *)&timerEventLists);
}

static void _timer_callback(union sigval v)
{
	tEventLists *t = NULL;
	struct itimerspec its;
	
	t = (tEventLists *)v.sival_ptr;
	if(t)
	{
		t->overrun++;
		if(t->func && (t->count == 0 || t->overrun <= t->count)){
			t->func(t->data);
		}
		if(t->count != 0)
		{
			if(t->overrun < t->count)
			{
				if(t->timerid && do_runTimer)
				{
					memset(&its, 0, sizeof(struct itimerspec));
					its.it_value.tv_sec = t->sec;
					its.it_value.tv_nsec = t->nsec;
					
					if(timer_settime(t->timerid, 0, &its, NULL) == -1)
					{
						perror("timer_settime: ");
						del_timer_event(t);
						return;
					}
				}
			}
			else del_timer_event(t);
		}
	}
}

tEventLists* add_timer_event(int id, int sec, int nsec, int count, tFunc func, void *data, int data_len)
{
	tEventLists *t = NULL;
	
	t = (tEventLists *)calloc(1, sizeof(tEventLists));
	if(t == NULL)
	{
		perror("calloc: ");
		goto error;
	}
	tlist_node_init((tList_s *)t);
	t->id = id;
	if(data) 
	{
		if(data_len > 0)
		{
			t->data = malloc(data_len);
			if(t->data == NULL)
			{
				perror("malloc: ");
				goto error;
			}
			memcpy(t->data, data, data_len);
			t->data_len = data_len;
		}
		else t->data = data;
	}
	t->count = count;
	t->sec = sec;
	t->nsec = nsec;
	if(func) t->func = func;
	tlist_add((tList_s *)&timerEventLists, (tList_s *)t);
	
	if(!do_runTimer) return t;
	
	if(start_timer_event(t) != 0)
		goto error;
	
    return t;
	
error:
	del_timer_event(t);
	return NULL;
}

int del_timer_event(tEventLists *t)
{
	int ret = 1;
	if(t != NULL)
	{
		tlist_remove((tList_s *)t);
		if(t->timerid && timer_delete(t->timerid))
			perror("timer_delete: ");
		if(t->data_len) free(t->data);
		free(t);
		ret = 0;
	}
	return ret;
}

tEventLists* get_timer_event(int id)
{
	tEventLists *t;
	t = (tEventLists *)tlist_next((tList_s *)&timerEventLists);
	while(t != NULL) 
	{
		if(t->id == id)
			return t;
		t = (tEventLists *)tlist_next((tList_s *)t);
	}
	return NULL;
}

int stop_timer_event(tEventLists *t)
{
	if(t)
	{
		if(t->timerid)
		{
			timer_gettime(t->timerid, &t->oldits);
			if(timer_delete(t->timerid))
				perror("timer_delete: ");
			else t->timerid = 0;
		}
	}
	return 0;
}

int start_timer_event(tEventLists *t)
{
	timer_t timerid;
	struct sigevent sev;
    struct itimerspec its;
	
	if(t)
	{
		if(t->timerid == 0)
		{
			memset(&sev, 0, sizeof(struct sigevent));
			memset(&its, 0, sizeof(struct itimerspec));
			sev.sigev_value.sival_ptr = (void*)t;
			sev.sigev_notify = SIGEV_THREAD;
			sev.sigev_notify_function = _timer_callback;

			if(timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1)
			{
				perror("timer_create: ");
				return -1;
			}
			t->timerid = timerid;
			
			if(t->oldits.it_value.tv_sec > 0 || t->oldits.it_value.tv_nsec > 0){
				its.it_value.tv_sec = t->oldits.it_value.tv_sec;
				its.it_value.tv_nsec = t->oldits.it_value.tv_nsec;
				its.it_interval.tv_sec = t->oldits.it_interval.tv_sec;
				its.it_interval.tv_nsec = t->oldits.it_interval.tv_nsec;
				memset(&t->oldits, 0, sizeof(struct itimerspec));
			}
			else
			{
				its.it_value.tv_sec = t->sec;
				its.it_value.tv_nsec = t->nsec;

				if(t->count == 0) //loop event
				{
					its.it_interval.tv_sec = its.it_value.tv_sec;
					its.it_interval.tv_nsec = its.it_value.tv_nsec;
				}
			}
			if(timer_settime(t->timerid, 0, &its, NULL) == -1)
			{
				perror("timer_settime: ");
				return -1;
			}
		}
	}
	return 0;
}

int stop_and_run_timer_event(tEventLists *t)
{
	if(t)
	{
		stop_timer_event(t);
		t->overrun++;
		if(t->func && (t->count == 0 || t->overrun <= t->count))
			t->func(t->data);
	}
	return 0;
}

int stop_all_timer_evenvt(void)
{
	tEventLists *t;
	do_runTimer = 0;
	t = (tEventLists *)tlist_next((tList_s *)&timerEventLists);
	while(t != NULL) 
	{
		stop_timer_event(t);
		t = (tEventLists *)tlist_next((tList_s *)t);
	}
	return 0;
}

int start_all_timer_evenvt(void)
{
	tEventLists *t;
	do_runTimer = 1;
	t = (tEventLists *)tlist_next((tList_s *)&timerEventLists);
	while(t != NULL) 
	{
		start_timer_event(t);
		t = (tEventLists *)tlist_next((tList_s *)t);
	}
	return 0;
}
