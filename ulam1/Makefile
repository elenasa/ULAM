####
# Top level Makefile driver for ULAM

include VERSION.mk

####
# TOP_TARGETS: Targets that are handled at this level
TOP_TARGETS:=
TOP_TARGETS+=doc       # Build documentation in doc/ref
TOP_TARGETS+=dist      # Export as tar file
TOP_TARGETS+=realclean # Deep cleaning: Nuke build, bin, doc/ref, also clean
TOP_TARGETS+=projini   # Initialize CWD as an ULAM project, if it isn't already


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

testclean:	FORCE
	rm -f bin/test
#	rm -f build/test/test.inc
	rm -f build/drivers/test/main*
	$(MAKE) -C src/test all
	$(MAKE) -C src/drivers

clean:	FORCE
	@rm -f *~

BASENAME=$(shell basename `pwd`)
dist:	realclean
	cd ..;tar cfz ulam${ULAM_LANGUAGE_VERSION}-$(BUILD_DATE_TIME).tgz $(BASENAME)

####
# Variables exported to submakes

# Root of source tree
ROOT_DIR := $(shell pwd)
export ROOT_DIR

export SRC_TARGETS

export ULAM_VERSION_MAJOR
export ULAM_VERSION_MINOR
export ULAM_VERSION_REV

export ULAM_LANGUAGE_VERSION
ULAM_LANGUAGE_VERSION:=1

# Defines from top level
export DEFINES
export ULAM_BUILD_DATE := $(shell date -u +%Y%m%d)
export ULAM_BUILD_TIME := $(shell date -u +%H%M%S)
export ULAM_BUILD_TIMESTAMP := $(ULAM_BUILD_DATE)$(ULAM_BUILD_TIME)
export ULAM_BUILD_WHO:=$(shell whoami)
export ULAM_BUILD_WHERE:=$(shell hostname)
export ULAM_TREE_VERSION:=$(shell cd $(ROOT_DIR) && git describe || echo "unknown-rev")

DEFINES+=-DBUILD_DATE="0x$(ULAM_BUILD_DATE)" -DBUILD_TIME="0x$(ULAM_BUILD_TIME)" -DBUILD_DATE_TIME="$(ULAM_BUILD_TIMESTAMP)"
DEFINES+=-DULAM_VERSION_MAJOR=$(ULAM_VERSION_MAJOR) -DULAM_VERSION_MINOR=$(ULAM_VERSION_MINOR) -DULAM_VERSION_REV=$(ULAM_VERSION_REV)
DEFINES+=-DULAM_TREE_VERSION="$(ULAM_TREE_VERSION)"

# Compilation flags from top level
export CFLAGS
CFLAGS+=-g2 -ansi -Wall -pedantic -Werror -Wformat

# Libs from top level
export LIBS

# Root of shared files tree.
ifeq ($(ULAM_SHARE_DIR),)
  ULAM_SHARE_DIR:=$(ROOT_DIR)/share
endif
export ULAM_SHARE_DIR

# Root of MFM simulator tree (configure in Makefile.local.mk if known)
export MFM_ROOT_DIR

# MFM version we're built on
export MFM_VERSION_NUMBER

# MFM git rev we're built on
export MFM_TREE_VERSION

# Things that all builds should depend on
export ALLDEP
ALLDEP += $(shell find $(ROOT_DIR) -name 'Makefile')

# Allow local configuration, if any, to override us:
-include Makefile.local.mk

ifeq ($(MFM_ROOT_DIR),)
${warning MFM_ROOT_DIR not configured}
${info  =>Consider creating '$(ROOT_DIR)/Makefile.local.mk', containing something like}
${info  =>  MFM_ROOT_DIR := /path/to/source/dir/of/MFMv2}
${info  =>to avoid this message}
else
${info Local configuration: MFM_ROOT_DIR=$(MFM_ROOT_DIR)}
## This wants to be rethought and refactored!
COMPONENTNAME:=ulamic
MFM_TARGET:=linux
BASEDIR:=$(MFM_ROOT_DIR)
include $(BASEDIR)/config/Makecommon.mk
CFLAGS += -I $(BASEDIR)/src/core/include
CFLAGS += -D ULAM_SHARE_DIR=$(ULAM_SHARE_DIR)
LIBS := -L $(BASEDIR)/build/core $(LIBS)
${info (top-level) LIBS='$(LIBS)'}
endif

####
# Top level rules

$(SRC_TARGETS):	src

# src rule just passes the make goals down to src
src:	FORCE
	@$(MAKE) -C $@ $(MAKECMDGOALS)

# pattern rule to build shared libs of ulam elements destined to live
# inside the MFM tree.  NOTE: MAKE BOTH TREES FIRST!
ULAMDIR:=share/ulam
ULAMWORKDIR:=.gen
$(MFM_ROOT_DIR)/res/elements/libue%.so:	$(ULAMDIR)/%/*.ulam
	./bin/ulam -lo --sd $(ULAMDIR)/core --sd $(ULAMDIR)/$* $(^:$(ULAMDIR)/$*/%=%)
	mv -f $(ULAMWORKDIR)/bin/libcue.so "$@"
	rm -rf $(ULAMWORKDIR)

ULAM_MFM_TARGETS+=$(MFM_ROOT_DIR)/res/elements/libuecore.so
ULAM_MFM_TARGETS+=$(MFM_ROOT_DIR)/res/elements/libuedemos.so
ulamexports:	$(ULAM_MFM_TARGETS)

cleanulamexports:	FORCE
	rm -f $(ULAM_MFM_TARGETS)

# Shortcut to ulam template build
ulam:	FORCE
	@$(MAKE) -C src/drivers/ulam

.PHONY:	$(TOP_TARGETS) $(SRC_TARGETS) FORCE
