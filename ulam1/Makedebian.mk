# Extra stuff for debian install
# Note this is meant to be run as 'make -C Makedebian.mk install'!
#
# Strategy 20150713: This is completely and obviously awful (if also
# painfully typical) but it appears a seriously non-finite task to do
# anything else, so we're basically going to put the entire MFM tree
# under /usr/lib/ulam/MFM.  ulam needs a _lot_ (though strictly
# speaking not _all_) of the MFM tree at _runtime_, including
# executables plus .h's, .tcc's, .inc's, and Makefiles.  At the
# moment, those are all intertwined under MFM_ROOT_DIR relative
# references -- which this way, at least for now, will continue to
# work.  Then we do symlinks in /usr/bin for the programs.

DEB_COMBINED_ROOT_DIR := $(DESTDIR)/usr/lib/ulam/
DEB_SYMLINK_BIN_DIR := $(DESTDIR)/usr/bin

install:	FORCE
	echo KDKDINSTALL
	install -d $(DEB_COMBINED_ROOT_DIR)
	./share/perl/extractDistro.pl all ../MFM ../ULAM $(DEB_COMBINED_ROOT_DIR)
	./share/perl/installSymlinks.pl `pwd` $(DEB_SYMLINK_BIN_DIR)

include VERSION.mk
version:	FORCE
	@echo $(ULAM_VERSION_NUMBER)

.PHONY:	FORCE
