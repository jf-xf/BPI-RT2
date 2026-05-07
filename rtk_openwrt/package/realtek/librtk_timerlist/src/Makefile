###############################################################################################################
#
#This lib is created by Realtek. It is a timer tool, you can use it.
#
###############################################################################################################

RTK_TIMER_LIST_SRC=timerEventList.c
RTK_TIMER_LIST_HEADER=timerEventList.h
LIBRTK_TIMERLIST_SO=librtk_timerlist.so
OBJS=$(RTK_TIMER_LIST_SRC:.c=.o)
LDFLAGS=-lrt


all: $(OBJS)
	$(CC) -shared -o $(LIBRTK_TIMERLIST_SO) $(OBJS)

install-dev:
	mkdir -p $(DESTDIR)/include/rtk
	cp -f $(RTK_TIMER_LIST_HEADER) $(DESTDIR)/include/rtk
	mkdir -p $(DESTDIR)/usr/lib
	cp -af $(LIBRTK_TIMERLIST_SO) $(DESTDIR)/usr/lib

install:
	mkdir -p $(DESTDIR)/usr/lib
	cp -af $(LIBRTK_TIMERLIST_SO) $(DESTDIR)/usr/lib

.c.o:
	$(CC) $(LDFLAGS) -Wall -g -fPIC -Werror -o $(@) -c $<

clean:
	rm -rf *.o *.so


.PHONY: all clean install-dev install
