ifneq ($(strip $(shell firmtool -v 2>&1 | grep usage)),)
$(error "firmtoolが無いです")
endif

SUBFOLDERS	:=	arm11 arm9

.PHONY:	all $(SUBFOLDERS)

all:		boot.firm

boot.firm:	$(SUBFOLDERS)
	@firmtool build $@ -D arm11/arm11.elf arm9/arm9.elf -A 0x18180000 -C XDMA XDMA NDMA XDMA
	@echo built... $(notdir $@)

# arm11 arm9
$(SUBFOLDERS):
	@$(MAKE) -C $@ all
