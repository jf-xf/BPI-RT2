DIRECTORY := core tools

.PHONY: all
all:
	$(info $(DIRECTORY))
	$(foreach dir, $(DIRECTORY), $(MAKE) -C $(dir) || exit;)

.PHONY: romfs
romfs:
	$(info $(DIRECTORY))
	$(foreach dir, $(DIRECTORY), $(MAKE) -C $(dir) $@;)
	$(ROMFSINST) multiap.conf /etc/multiap.conf

.PHONY: clean
clean:
	$(info $(DIRECTORY))
	$(foreach dir, $(DIRECTORY), $(MAKE) -C $(dir) $@;)