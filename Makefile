all:	FORCE
	@if [ -e MFM ]; then COMMANDS=1 make -C MFM all ; fi
	COMMANDS=1 make -C ulam1 all
	COMMANDS=1 make -C ulam1 ulamexports

install:	FORCE
	make -C ulam1 -f Makedebian.mk install

.PHONY:	FORCE
