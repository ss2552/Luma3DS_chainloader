SUBFOLDERS	:=	arm11 arm9

.PHONY:	all $(SUBFOLDERS)

all:		boot.firm

boot.firm:	 $(SUBFOLDERS)
	@firmtool build $@ -D arm11/arm11.elf arm9/arm9.elf -A 0x18180000 -C XDMA NDMA
	@echo built... $(notdir $@)

$(SUBFOLDERS):
	@$(MAKE) -C $@ all
