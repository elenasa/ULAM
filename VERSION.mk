####
# THE ULAM COMPILER VERSION IS DEFINED HERE
#
# Changes to this file should be committed alone, and then HEAD should
# be tagged, and the tag pushed, something like this:
#$ git tag -a -m "Tagging v1.0.0" v1.0.0
#$ git push
#$ git push origin v1.0.0

ULAM_VERSION_MAJOR:=5
ULAM_VERSION_MINOR:=0
ULAM_VERSION_REV:=2

################## NOTHING BELOW HERE SHOULD NEED TO CHANGE ##################

ULAM_VERSION_NUMBER:=$(ULAM_VERSION_MAJOR).$(ULAM_VERSION_MINOR).$(ULAM_VERSION_REV)

# Get root dir now if we don't have it yet
ifeq ($(ULAM_ROOT_DIR),)
ULAM_ROOT_DIR := $(shell pwd)
export ULAM_ROOT_DIR
endif

# Suck in a git rep marker if it's been cached
ULAM_TREE_VERSION:=unknown-rev
-include $(ULAM_ROOT_DIR)/ULAM_TREEVERSION.mk

# If our dir is writable, and we have git, and there's a repo tag,
# that means we are in the ULAM_REPO_BUILD_TIME era, so we should cache
# the tag for use in later eras.
SHOULD_CACHE_REPO_TAG_ULAM:=$(shell test -w $(ULAM_ROOT_DIR) && which git >/dev/null && cd $(ULAM_ROOT_DIR) && git describe >/dev/null 2>&1 && echo YES)
#${info AT<<$(realpath $(ULAM_ROOT_DIR))>>=($(SHOULD_CACHE_REPO_TAG_ULAM))}
ifeq ($(SHOULD_CACHE_REPO_TAG_ULAM),YES)
  ULAM_TREE_VERSION:=$(shell cd $(ULAM_ROOT_DIR) && git describe)
  $(shell echo "ULAM_TREE_VERSION:=$(ULAM_TREE_VERSION)" > $(ULAM_ROOT_DIR)/ULAM_TREEVERSION.mk)
else
endif
export ULAM_TREE_VERSION
