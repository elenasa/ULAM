####
# Top level Makefile driver for ULAM

# Root of source tree
ULAM_ROOT_DIR := $(shell pwd)
export ULAM_ROOT_DIR

include VERSION.mk

####
# TOP_TARGETS: Targets that are handled at this level
TOP_TARGETS:=
TOP_TARGETS+=doc       # Build documentation in doc/ref
TOP_TARGETS+=dist      # Export as tar file
TOP_TARGETS+=topclean  # Light cleaning in top dir only
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
realclean:	topclean
	rm -rf $(REALCLEAN_DIRS)
	mkdir -p  $(REALCLEAN_DIRS)
	make clean

testclean:	FORCE
	rm -f bin/test
#	rm -f build/test/test.inc
	rm -f build/drivers/test/main*
	$(MAKE) -C src/test all
	$(MAKE) -C src/drivers

topclean:	FORCE
	@rm -f *~

BASENAME=$(shell basename `pwd`)
dist:	realclean
	cd ..;tar cfz ulam${ULAM_LANGUAGE_VERSION}-$(BUILD_DATE_TIME).tgz $(BASENAME)

####
# Variables exported to submakes

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

DEFINES+=-DBUILD_DATE="0x$(ULAM_BUILD_DATE)" -DBUILD_TIME="0x$(ULAM_BUILD_TIME)" -DBUILD_DATE_TIME="$(ULAM_BUILD_TIMESTAMP)"
DEFINES+=-DULAM_VERSION_MAJOR=$(ULAM_VERSION_MAJOR) -DULAM_VERSION_MINOR=$(ULAM_VERSION_MINOR) -DULAM_VERSION_REV=$(ULAM_VERSION_REV)
DEFINES+=-DULAM_TREE_VERSION="$(ULAM_TREE_VERSION)"

# Compilation flags from top level
export CFLAGS
CFLAGS+=-g2 -ansi -Wall -pedantic -Werror -Wformat
#CFLAGS+=-DNDEBUG

# Libs from top level
export LIBS

# Root of shared files tree.
ifeq ($(ULAM_SHARE_DIR),)
  ULAM_SHARE_DIR:=$(ULAM_ROOT_DIR)/share
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
ALLDEP += $(shell find $(ULAM_ROOT_DIR) -name 'Makefile')

# Allow local configuration, if any, to override us:
-include Makefile.local.mk

ifeq ($(MFM_ROOT_DIR),)
# check installed location
ifeq ("$(wildcard ../MFM)","")
${warning MFM_ROOT_DIR not configured}
${info  =>Consider creating '$(ULAM_ROOT_DIR)/Makefile.local.mk', containing something like}
${info  =>  MFM_ROOT_DIR := /path/to/source/dir/of/MFMv2}
${info  =>to avoid this message}
else
MFM_ROOT_DIR:=$(realpath ../MFM)
endif
endif
#${info Local configuration: MFM_ROOT_DIR=$(MFM_ROOT_DIR)}
## This wants to be rethought and refactored!
COMPONENTNAME:=ulamic
MFM_TARGET:=linux
BASEDIR:=$(MFM_ROOT_DIR)
include $(BASEDIR)/config/Makecommon.mk
override CFLAGS += -I $(BASEDIR)/src/core/include
override CFLAGS += -D ULAM_SHARE_DIR=$(ULAM_SHARE_DIR)
override LIBS := -L $(BASEDIR)/build/core $(LIBS)

####
# Top level rules

$(SRC_TARGETS):	src

# src rule just passes the make goals down to src
src:	FORCE
	@$(MAKE) -k -C $@ $(MAKECMDGOALS)

# pattern rule to build shared libs of ulam elements destined to live
# inside the MFM tree.  NOTE: MAKE BOTH TREES FIRST!
ULAMDIR:=share/ulam
ULAMWORKDIR:=.gen
ULAMDEMODATE:=$(shell date +%Y%m%d%H%M%S)
ULAMDEMOVERSION:=$(shell if [ -x ./bin/ulam ] ; then ./bin/ulam -V 2>&1 ; else echo --unknown version-- ; fi)
ULAMDEMOWHO:=$(shell whoami)
ULAMDEMOKEY:="MFM-DEMOS-$(ULAMDEMODATE)-$(ULAMDEMOVERSION)-$(ULAMDEMOWHO)"
MFZMAKEPATH:=$(MFM_ROOT_DIR)/bin/mfzmake
makedemokey:	FORCE
	$(MFZMAKEPATH) keygen $(ULAMDEMOKEY)

burndemokey:	FORCE
	$(MFZMAKEPATH) burn $(ULAMDEMOKEY)

havedemokey:	FORCE
	$(MFZMAKEPATH) cansign $(ULAMDEMOKEY)

donothavedemokey:	FORCE
	$(MFZMAKEPATH) keygen $(ULAMDEMOKEY)
	$(MFZMAKEPATH) cansign $(ULAMDEMOKEY) 2>/dev/null || exit 0

$(MFM_ROOT_DIR)/res/elements/libue%.so:	$(ULAMDIR)/%/*.ulam
	./bin/ulam -lo --sd $(ULAMDIR)/core --sd $(ULAMDIR)/$* $(^:$(ULAMDIR)/$*/%=%)
	mv -f $(ULAMWORKDIR)/bin/libcue.so "$@"
	rm -rf $(ULAMWORKDIR)

$(MFM_ROOT_DIR)/res/elements/demos/libue%.so:	$(ULAMDIR)/demos/%/*.ulam
	mkdir -p $(MFM_ROOT_DIR)/res/elements/demos
	./bin/ulam --sc -lo --sd $(ULAMDIR)/core --sd $(ULAMDIR)/demos/$* $(^:$(ULAMDIR)/demos/$*/%=%)
	mv -f $(ULAMWORKDIR)/bin/libcue.so "$@"
	rm -rf $(ULAMWORKDIR)

$(MFM_ROOT_DIR)/res/elements/demos/%.mfz:	$(ULAMDIR)/demos/%/*.ulam
	mkdir -p $(MFM_ROOT_DIR)/res/elements/demos
	./bin/ulam --sc -z $(ULAMDEMOKEY) -o --sd $(ULAMDIR)/core --sd $(ULAMDIR)/demos/$* $(^:$(ULAMDIR)/demos/$*/%=%)
	mv -f ./a.mfz "$@"
	rm -rf $(ULAMWORKDIR)

ULAM_MFM_TARGETS+=$(MFM_ROOT_DIR)/res/elements/libuecore.so
ULAM_DEMO_PATHS:=$(wildcard $(ULAMDIR)/demos/*/.)
ULAM_DEMO_DIRS:=$(patsubst %/.,%,$(ULAM_DEMO_PATHS))
ULAM_DEMO_NAMES:=$(patsubst $(ULAMDIR)/demos/%,%,$(ULAM_DEMO_DIRS))
ULAM_DEMO_LIBS:=$(patsubst %,$(MFM_ROOT_DIR)/res/elements/demos/libue%.so,$(ULAM_DEMO_NAMES))
ULAM_MFM_TARGETS+=$(ULAM_DEMO_LIBS)
ULAM_DEMO_MFZS:=$(patsubst %,$(MFM_ROOT_DIR)/res/elements/demos/%.mfz,$(ULAM_DEMO_NAMES))

ulamexports:	$(ULAM_MFM_TARGETS) | makedemokey $(ULAM_DEMO_MFZS) burndemokey # $(info  (ULAM_MFM_TARGETS) [$(ULAM_MFM_TARGETS)]) $(info  (ULAM_DEMO_DIRS) [$(ULAM_DEMO_DIRS)]) $(info  (ULAM_DEMO_NAMES) [$(ULAM_DEMO_NAMES)]) $(info  (ULAM_DEMO_LIBS) [$(ULAM_DEMO_LIBS)]) $(info  (ULAM_DEMO_MFZS) [$(ULAM_DEMO_MFZS)]) $(info  (ULAM_DEMO_PATHS) [$(ULAM_DEMO_PATHS)])

cleanulamexports:	FORCE
	rm -f $(ULAM_MFM_TARGETS) $(ULAM_DEMO_MFZS)

# Shortcut to ulam template build
ulam:	FORCE
	@$(MAKE) -C src/drivers/ulam

.PHONY:	$(TOP_TARGETS) $(SRC_TARGETS) FORCE
