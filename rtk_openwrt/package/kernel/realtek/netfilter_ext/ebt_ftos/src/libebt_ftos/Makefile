
PWD := $(shell pwd)
LIBS = libebt_ftos.so
OBJS = libebt_ftos.o

CFLAGS += -Wall -fPIC -I$(PWD)/../include
CFLAGS	+= -I$(INCDIR)/usr/include/ -I$(INCDIR)/usr/include/ebtables

all: $(LIBS)

.c.o: 
	$(CC) -M $(CFLAGS) $< > $@.d
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $^

clean:
	rm -f $(LIBS) $(LIBS1) *.o *.d

install:
	mkdir -p $(DESTDIR)/usr/local/lib/ebtables
	cp -af $(LIBS) $(DESTDIR)/usr/local/lib/ebtables
