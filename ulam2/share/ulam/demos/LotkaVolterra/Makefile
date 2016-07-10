DIR:=$(shell pwd)
DEMO:=$(shell basename "$(DIR)")
DEPS:=--sd ../../misc

all:	run

mfz:	$(DEMO).mfz

run:	$(DEMO).mfz
	mfzrun $(DEMO).mfz

$(DEMO).mfz:	*.ulam
	../../../../bin/ulam $(DEPS) $^ $@

clean:
	rm -f $(DEMO).mfz
