# SUBFOLDERS	:=	arm11 arm9

.PHONY:	arm9 # all $(SUBFOLDERS)

# all:		boot.firm

# boot.firm:	arm11 # $(SUBFOLDERS)
#	@firmtool build $@ -D arm11/arm11.elf arm9/arm9.elf -A 0x18180000 -C XDMA XDMA NDMA XDMA
#	@echo built... $(notdir $@)

# arm11 arm9
# $(SUBFOLDERS):
arm9:
	@$(MAKE) -C $@ all
