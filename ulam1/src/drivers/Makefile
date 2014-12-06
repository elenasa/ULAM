####
# SUB_DIRS listed in the order they should be built (bottom up)

SUB_DIRS:=
SUB_DIRS+=culam
SUB_DIRS+=eulam
SUB_DIRS+=test
SUB_DIRS+=ulam

####
# Assume all drivers depend on ulam..
ALLDEP+=$(ROOT_DIR)/src/ulam

####
# We are defining BASEDIR relative to our child dirs because they're
# the ones that will use it and we suck!
BASEDIR:=../../..
ULAM_LIB_DIR:=$(BASEDIR)/build/ulam
# Include our library at the front and the back because we
# are utterly lame-o-tronic
LIBS := -L $(ULAM_LIB_DIR) -lulam $(LIBS) -lulam

$(SRC_TARGETS):	$(SUB_DIRS)

$(SUB_DIRS):	FORCE
	@$(MAKE) -C $@ $(MAKECMDGOALS)  # Pass our make goal down to each subdirectory

clean:	FORCE
	@rm -f *~

.PHONY:	FORCE
