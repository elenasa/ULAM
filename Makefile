all:	FORCE
	make -C ulam1 all

install:	FORCE
	make -C ulam1 -f Makedebian.mk install

.PHONY:	FORCE
