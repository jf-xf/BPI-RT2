#ifndef _TIMER_EVENT_LIST_
#define _TIMER_EVENT_LIST_
#include <time.h>

#ifndef TRUE 
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef void (*tFunc) (void *);
typedef void (*TLIST_DESTFUNC)(void *);

typedef struct tList_t {
	unsigned char headFlag;
	struct tList_t *prev;
	struct tList_t *next;
}tList_s;

void tlist_header_init(tList_s *);
void tlist_node_init(tList_s *);
int tlist_size(tList_s *);
tList_s *tlist_get(tList_s *, int);
tList_s *tlist_getlast(tList_s *);
void tlist_insert( tList_s *, tList_s *);
void tlist_add(tList_s *, tList_s *);
void tlist_remove(tList_s *);
tList_s *tlist_prev(tList_s *);
tList_s *tlist_next(tList_s *);
void tlist_clear(tList_s *, TLIST_DESTFUNC);

typedef struct _tEventLists {
	unsigned char headFlag;
	struct _tEventLists *prev;
	struct _tEventLists *next;
	int id;
	tFunc func;
	void *data;
	int data_len;
	timer_t timerid;
	unsigned int sec;
	unsigned int nsec;
	struct itimerspec oldits;
	int count;
	unsigned int overrun;
}tEventLists;

void init_timer_event(void);
tEventLists* add_timer_event(int id, int sec, int nsec, int count, tFunc func, void *data, int data_len);
int del_timer_event(tEventLists *t);
tEventLists* get_timer_event(int id);
int stop_timer_event(tEventLists *t);
int start_timer_event(tEventLists *t);
int stop_and_run_timer_event(tEventLists *t);
int stop_all_timer_evenvt(void);
int start_all_timer_evenvt(void);
int get_timer_event_size(void);

#endif
