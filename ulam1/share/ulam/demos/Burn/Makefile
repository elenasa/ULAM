DEMO:=Burn
run:	$(DEMO).mfz
	mfzrun $(DEMO).mfz

$(DEMO).mfz:	*.ulam
	../../../../bin/ulam --sd ../../misc $^ $@
