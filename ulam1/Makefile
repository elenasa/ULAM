####
# Top level Makefile driver for ULAM


####
# TOP_TARGETS: Targets that are handled at this level
TOP_TARGETS:=
TOP_TARGETS+=doc       # Build documentation in doc/ref
TOP_TARGETS+=dist      # Export as tar file
TOP_TARGETS+=realclean # Deep cleaning: Nuke build, bin, doc/ref, also clean


####
# SRC_TARGETS: Targets that are passed to src/
SRC_TARGETS:=
SRC_TARGETS+=all       # Build everything that needs it
SRC_TARGETS+=clean     # Light cleaning: ~'s .o's and such

# Default target
all:

####
# Targets handled at top level

doc:	FORCE
	mkdir -p doc/ref
	echo doxygen

REALCLEAN_DIRS:=build bin doc/ref
realclean:	FORCE
	rm -rf $(REALCLEAN_DIRS)
	mkdir -p  $(REALCLEAN_DIRS)
	$(MAKE) clean

clean:	FORCE
	@rm -f *~

BASENAME=$(shell basename `pwd`)
dist:	realclean
	cd ..;tar cfz ulam${ULAM_LANGUAGE_VERSION}-$(BUILD_DATE_TIME).tgz $(BASENAME)

####
# Variables exported to submakes

export SRC_TARGETS

export ULAM_LANGUAGE_VERSION
ULAM_LANGUAGE_VERSION:=1

# Compilation flags from top level
export CFLAGS
CFLAGS+=-g

# Root of source tree
ROOT_DIR := $(shell pwd)
export ROOT_DIR

# Things that all builds should depend on
export ALLDEP
ALLDEP += $(shell find $(ROOT_DIR) -name 'Makefile')

export BUILD_DATE BUILD_TIME BUILD_DATE_TIME
BUILD_DATE := $(shell date -u +%Y%m%d)
BUILD_TIME := $(shell date -u +%H%M%S)
BUILD_DATE_TIME := $(BUILD_DATE)$(BUILD_TIME)

####
# Top level rules

$(SRC_TARGETS):	src

# src rule just passes the make goals down to src
src:	FORCE
	@$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY:	$(TOP_TARGETS) $(SRC_TARGETS) FORCE
