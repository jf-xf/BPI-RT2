%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FPIC) -Wall -c -o $@ $<

clean:
	rm -f rtk-luci-bwc *.o

rtk-luci-bwc: luci-bwc.o
	$(CC) $(LDFLAGS) -o $@ $^ -ldl

compile: rtk-luci-bwc

install: compile
	mkdir -p $(DESTDIR)/usr/bin
	cp rtk-luci-bwc $(DESTDIR)/usr/bin/rtk-luci-bwc
