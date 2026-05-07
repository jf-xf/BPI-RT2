

EXEC = smuxctl
OBJS = smuxctl.o smuxctl_test_case.o

CFLAGS += -Wall -I../module/src
ifeq ($(OPENWRT_BUILD), 1)
  CFLAGS += -include $(LINUXDIR)/include/generated/autoconf.h
  CFLAGS += -include $(ROOTDIR)/autoconf.h
  CFLAGS += -I$(INCDIR)/include -I$(INCDIR)/include/rtk
else
  CFLAGS += -include $(ROOTDIR)/$(LINUXDIR)/include/generated/autoconf.h
  CFLAGS += -I$(ROOTDIR)/include -I$(ROOTDIR)/include/rtk
endif

all: $(EXEC) 

.c.o: 
	$(CC) -M $(CFLAGS) $< > $@.d
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm $(EXEC) *.o *.d

install:
	mkdir -p $(DESTDIR)/bin
	cp -af $(EXEC) $(DESTDIR)/bin
