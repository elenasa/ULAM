####
# SUB_DIRS listed in the order they should be built (bottom up)

SUB_DIRS:=
SUB_DIRS+=culam
SUB_DIRS+=eulam
SUB_DIRS+=test

$(SRC_TARGETS):	$(SUB_DIRS)

$(SUB_DIRS):	FORCE
	@$(MAKE) -C $@ $(MAKECMDGOALS)  # Pass our make goal down to each subdirectory

clean:	FORCE
	@rm -f *~

.PHONY:	FORCE
