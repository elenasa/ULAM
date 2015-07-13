# Extra stuff for debian install
# Note this is meant to be run as 'make -C Makedebian.mk install'!
DEB_ULAM_PROGRAMS_TO_INSTALL += ulam culam
DEB_ULAM_PROGRAMS_PATHS_TO_INSTALL := $(DEB_ULAM_PROGRAMS_TO_INSTALL:%=bin/%)

DEB_MFM_PROGRAMS_TO_INSTALL += mfms mfzmake mfzrun
DEB_MFM_PROGRAMS_PATHS_TO_INSTALL := $(DEB_MFM_PROGRAMS_TO_INSTALL:%=../MFM/bin/%)

DEB_ULAM_BINDIR := $(DESTDIR)/usr/bin
DEB_ULAM_SHAREDIR := $(DESTDIR)/usr/share/ulam/

DEB_MFM_BINDIR := $(DESTDIR)/usr/bin
DEB_MFM_SHAREDIR := $(DESTDIR)/usr/share/mfm/

# All the the stuff we need from MFM to do ulam compiles..
DEB_ULAM_MFM_PATHS += $(wildcard ../MFM/build/*/lib*.a)
DEB_ULAM_MFM_PATHS += $(wildcard ../MFM/config/*.mk)
DEB_ULAM_MFM_PATHS += ../MFM/src
DEB_ULAM_MFM_PATHS := $(patsubst ../%,%,$(DEB_ULAM_MFM_PATHS))

DEB_ULAM_MFM_DIR := $(DESTDIR)/usr/lib/ulam

# We're recursing rather than depending on 'all' so that the
# $(PLATFORMS) mechanism doesn't need to know about install.
install:	FORCE
	COMMANDS=1 SHARED_DIR=$(DEB_MFM_SHAREDIR) make -C ../MFM -k all
	COMMANDS=1 ULAM_SHARE_DIR=$(DEB_ULAM_SHAREDIR) make -k all
	mkdir -p $(DEB_ULAM_BINDIR)
	cp -a $(DEB_ULAM_PROGRAMS_PATHS_TO_INSTALL) $(DEB_ULAM_BINDIR)
	mkdir -p $(DEB_ULAM_SHAREDIR)
	cp -r share/* $(DEB_ULAM_SHAREDIR)
	cp -a $(DEB_MFM_PROGRAMS_PATHS_TO_INSTALL) $(DEB_ULAM_BINDIR)
	mkdir -p $(DEB_MFM_SHAREDIR)
	cp -r ../MFM $(DEB_MFM_SHAREDIR)
	mkdir -p $(DEB_ULAM_MFM_DIR)
	cd .. && cp --parents -r $(DEB_ULAM_MFM_PATHS) $(DEB_ULAM_MFM_DIR)
	# MAN AND DOC?

include VERSION.mk
version:	FORCE
	@echo $(ULAM_VERSION_NUMBER)

TAR_EXCLUDES+=--exclude=tools
TAR_EXCLUDES+=--exclude=*~
TAR_EXCLUDES+=--exclude=.git*
TAR_EXCLUDES+=--exclude=doc/internal
TAR_EXCLUDES+=--exclude=spike
TAR_EXCLUDES+=--exclude=spikes
TAR_EXCLUDES+=--exclude-backups
tar:	FORCE
	make realclean
	PWD=`pwd`;BASE=`basename $$PWD`;cd ../..;tar cvzf ulam-$(ULAM_VERSION_NUMBER).tgz $(TAR_EXCLUDES) *

.PHONY:	FORCE
