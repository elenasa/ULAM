####
# Top level Makefile driver for ULAM


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

export SRC_TARGETS

export ULAM_LANGUAGE_VERSION
ULAM_LANGUAGE_VERSION:=1

# Defines from top level
export DEFINES
BUILD_DATE := $(shell date -u +%Y%m%d)
BUILD_TIME := $(shell date -u +%H%M%S)
BUILD_DATE_TIME := $(BUILD_DATE)$(BUILD_TIME)
DEFINES+=-DBUILD_DATE="0x$(BUILD_DATE)" -DBUILD_TIME="0x$(BUILD_TIME)" -DBUILD_DATE_TIME="$(BUILD_DATE_TIME)"

# Compilation flags from top level
export CFLAGS
CFLAGS+=-g2 -ansi -Wall -pedantic -Werror -Wformat

# Libs from top level
export LIBS

# Root of source tree
ROOT_DIR := $(shell pwd)
export ROOT_DIR

# Root of shared files tree.
ifeq ($(ULAM_SHARE_DIR),)
  ULAM_SHARE_DIR:=$(ROOT_DIR)/share
endif
export ULAM_SHARE_DIR

# Root of MFM simulator tree (configure in Makefile.local.mk if known)
export MFM_ROOT_DIR

# MFM version we're built on
export MFM_VERSION_NUMBER

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
CFLAGS += $(DEFINES)
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

# Shortcut to ulam template build
ulam:	FORCE
	@$(MAKE) -C src/drivers/ulam

.PHONY:	$(TOP_TARGETS) $(SRC_TARGETS) FORCE
