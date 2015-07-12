# Extra stuff for debian install
# Note this is meant to be run as 'make -C Makedebian.mk install'!
DEB_PROGRAMS_TO_INSTALL += ulam culam
DEB_PROGRAMS_PATHS_TO_INSTALL := $(DEB_PROGRAMS_TO_INSTALL:%=bin/%)
DEB_ULAM_BINDIR := $(DESTDIR)/usr/bin
DEB_ULAM_SHAREDIR := $(DESTDIR)/usr/share/ulam/

# We're recursing rather than depending on 'all' so that the
# $(PLATFORMS) mechanism doesn't need to know about install.
install:	FORCE
	COMMANDS=1 ULAM_SHARE_DIR=$(DEB_ULAM_SHAREDIR) make -k all
	mkdir -p $(DEB_ULAM_BINDIR)
	cp -a $(DEB_PROGRAMS_PATHS_TO_INSTALL) $(DEB_ULAM_BINDIR)
	mkdir -p $(DEB_ULAM_SHAREDIR)
	cp -r share/* $(DEB_ULAM_SHAREDIR)
	# MAN AND DOC?

include VERSION.mk
version:	FORCE
	@echo $(ULAM_VERSION_NUMBER)

TAR_EXCLUDES+=--exclude=tools --exclude=*~ --exclude=.git --exclude=doc/internal --exclude=spikes --exclude-backups
tar:	FORCE
	make realclean
	PWD=`pwd`;BASE=`basename $$PWD`;cd ..;tar cvzf ulam-$(ULAM_VERSION_NUMBER).tgz $(TAR_EXCLUDES) $$BASE

.PHONY:	FORCE
