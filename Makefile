SUBFOLDERS	:=	arm11 arm9


.PHONY:	$(SUBFOLDERS)

$(SUBFOLDERS):
	@$(MAKE) -C $@ all
