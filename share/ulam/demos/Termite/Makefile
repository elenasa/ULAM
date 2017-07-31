ULAMBINDIR:=../../../../bin
TIMESTAMP:=$(shell date +%Y%m%d-%H%M%S)
DIR:=$(shell pwd)
DEMO:=$(shell basename "$(DIR)")
DEPS:=

all:	run

mfz:	$(DEMO).mfz

run:	$(DEMO).mfz
	mfzrun $(DEMO).mfz

$(DEMO).mfz:	*.ulam *.mfs
	$(ULAMBINDIR)/ulam $(DEPS) $^ $@

ishtar:	clean
	cd ..;tar cvzf ish-$(TIMESTAMP).tgz $(DEMO)
	echo $(TIMESTAMP)

clean:
	rm -f $(DEMO).mfz *~
	rm -rf .gen


