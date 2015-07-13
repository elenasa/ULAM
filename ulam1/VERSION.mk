####
# THE ULAM COMPILER VERSION IS DEFINED HERE
#
# Changes to this file should be committed alone, and then HEAD should
# be tagged, and the tag pushed, something like this:
#$ git tag -a -m "Tagging v1.0.0" v1.0.0
#$ git push origin v1.0.0

ULAM_VERSION_MAJOR:=1
ULAM_VERSION_MINOR:=0
ULAM_VERSION_REV:=0

################## NOTHING BELOW HERE SHOULD NEED TO CHANGE ##################

ULAM_VERSION_NUMBER:=$(ULAM_VERSION_MAJOR).$(ULAM_VERSION_MINOR).$(ULAM_VERSION_REV)

# Get the repo version (and save it for possible non-git-repo builds downstream)
HAVE_GIT_DESCRIBE:=$(shell cd $(ROOT_DIR) && git describe 2>&1 >/dev/null && echo $$?)
ifeq ($(HAVE_GIT_DESCRIBE),0)
  ULAM_TREE_VERSION:=$(shell cd $(ROOT_DIR) && git describe)
  $(shell echo "ULAM_TREE_VERSION:=$(ULAM_TREE_VERSION)" > TREEVERSION.mk)
else
  ULAM_TREE_VERSION:=unknown-rev
  -include TREEVERSION.mk
endif
export ULAM_TREE_VERSION
