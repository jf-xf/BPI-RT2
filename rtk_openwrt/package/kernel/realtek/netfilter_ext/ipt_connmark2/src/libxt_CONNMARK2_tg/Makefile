
PWD := $(shell pwd)
LIBS = libxt_CONNMARK2.so
OBJS = libxt_CONNMARK2.o

CFLAGS += -Wall -fPIC -I$(PWD)/../include
CFLAGS	+= -I$(INCDIR)/usr/include/iptables -I$(INCDIR)/usr/include

all: $(LIBS)

.c.o: 
	$(CC) -M $(CFLAGS) $< > $@.d
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBS): $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $^

clean:
	rm -f $(LIBS) $(LIBS1) *.o *.d

install:
	mkdir -p $(DESTDIR)/usr/local/lib/xtables
	cp -af $(LIBS) $(DESTDIR)/usr/local/lib/xtables
