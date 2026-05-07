#
# A wrapper Makefile for easymesh feature
#
.PHONY: clean romfs prepare vendor preconfig

all: preconfig
	$(MAKE) -f Makefile.build all || exit $?

clean:
	$(MAKE) -f Makefile.build clean || exit $?
	rm -f .config

romfs:
	$(MAKE) -f Makefile.build romfs || exit $?

prepare:
	$(MAKE) -f Makefile.build prepare || exit $?

vendor:
	$(MAKE) -f Makefile.build vendor || exit $?

preconfig:
	$(MAKE) -f Makefile.preconfig || exit $?
