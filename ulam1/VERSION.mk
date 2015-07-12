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
