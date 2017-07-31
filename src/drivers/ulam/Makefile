ifndef ULAM_ROOT_DIR
$(error ULAM_ROOT_DIR is unset.  (To make here, run 'make ulam' from the ulam tree toplevel))
endif
BASE_DIR:=../../..
DRIVER_NAME:=ulam
BIN_DIR:=$(BASE_DIR)/bin
SRC:=./$(DRIVER_NAME).tmpl
TARGET:=$(BIN_DIR)/$(DRIVER_NAME)
SRCS:=$(wildcard *.tmpl)
TARGETS:=$(patsubst %.tmpl, $(BIN_DIR)/%, $(SRCS))
ALLDEP+=./Makefile

all:	$(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/%: ./%.tmpl FORCE | $(BIN_DIR) # just always do this -- depends on ENV vars
	sed \
	    -e "s|@DEBIAN_PACKAGE_NAME@|$(DEBIAN_PACKAGE_NAME)|" \
	    -e "s|@CULAM_PATH@|$(ULAM_ROOT_DIR)/bin/culam|" \
	    -e "s|@ULAM_VERSION_MAJOR@|$(ULAM_VERSION_MAJOR)|" \
	    -e "s|@ULAM_VERSION_MINOR@|$(ULAM_VERSION_MINOR)|" \
	    -e "s|@ULAM_VERSION_REV@|$(ULAM_VERSION_REV)|" \
	    -e "s|@MFM_ROOT_DIR@|$(MFM_ROOT_DIR)|" \
	    -e "s|@MFM_VERSION_NUMBER@|$(MFM_VERSION_NUMBER)|" \
	    -e "s|@MFM_TREE_VERSION@|$(MFM_TREE_VERSION)|" \
	    -e "s|@ULAM_TREE_VERSION@|$(ULAM_TREE_VERSION)|" \
	    -e "s|@ULAM_SHARE_DIR@|$(ULAM_SHARE_DIR)|" \
	    -e "s|@ULAM_BUILD_TIMESTAMP@|$(ULAM_BUILD_TIMESTAMP)|" \
	    -e "s|@ULAM_BUILD_WHO@|$(ULAM_BUILD_WHO)|" \
	    -e "s|@ULAM_BUILD_WHERE@|$(ULAM_BUILD_WHERE)|" \
	    $< | \
	perl -e 'while(<>){if($$_=~/<<<INCLUDE:\s*(.+)\s*>>>/){system("cat $$1")} else {print}}' >$@
	chmod 755 $@


$(TARGET): $(SRC) FORCE | $(BIN_DIR) # just always do this -- depends on ENV vars
	sed \
	    -e "s|@CULAM_PATH@|$(ULAM_ROOT_DIR)/bin/culam|" \
	    -e "s|@ULAM_VERSION_MAJOR@|$(ULAM_VERSION_MAJOR)|" \
	    -e "s|@ULAM_VERSION_MINOR@|$(ULAM_VERSION_MINOR)|" \
	    -e "s|@ULAM_VERSION_REV@|$(ULAM_VERSION_REV)|" \
	    -e "s|@MFM_ROOT_DIR@|$(MFM_ROOT_DIR)|" \
	    -e "s|@MFM_VERSION_NUMBER@|$(MFM_VERSION_NUMBER)|" \
	    -e "s|@MFM_TREE_VERSION@|$(MFM_TREE_VERSION)|" \
	    -e "s|@ULAM_TREE_VERSION@|$(ULAM_TREE_VERSION)|" \
	    -e "s|@ULAM_SHARE_DIR@|$(ULAM_SHARE_DIR)|" \
	    -e "s|@ULAM_BUILD_TIMESTAMP@|$(ULAM_BUILD_TIMESTAMP)|" \
	    -e "s|@ULAM_BUILD_WHO@|$(ULAM_BUILD_WHO)|" \
	    -e "s|@ULAM_BUILD_WHERE@|$(ULAM_BUILD_WHERE)|" \
	    $< | \
	perl -e 'while(<>){if($$_=~/<<<INCLUDE:\s*(.+)\s*>>>/){system("cat $$1")} else {print}}' >$@
	chmod 755 $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

realclean:	clean
	@rm -f ${TARGET}

.PHONY:	FORCE
