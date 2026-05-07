include $(TOPDIR)/.config
include $(LINUX_DIR)/.config
include $(src)/platform.mk
include $(wildcard $(src)/platform/*.mk)

all:
	@echo "for install g6 conf"
