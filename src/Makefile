####
# SUB_DIRS listed in the order they should be built (bottom up)

SUB_DIRS:=
SUB_DIRS+=ulam
SUB_DIRS+=test
SUB_DIRS+=drivers

$(SRC_TARGETS):	$(SUB_DIRS)

$(SUB_DIRS):	FORCE
	@$(MAKE) -C $@ $(MAKECMDGOALS)  # Pass our make goal down to each subdirectory

clean:	FORCE
	@rm -f *~

.PHONY:	FORCE
