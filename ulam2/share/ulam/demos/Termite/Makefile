DIR:=$(shell pwd)
DEMO:=$(shell basename "$(DIR)")
DEPS:=

all:	run

mfz:	$(DEMO).mfz

run:	$(DEMO).mfz
	mfzrun $(DEMO).mfz run --ue-demos

$(DEMO).mfz:	*.ulam *.mfs
	ulam $(DEPS) $^ $@

clean:
	rm -f $(DEMO).mfz
