BASE_DIR:=../../..
DRIVER_NAME:=ulam
BIN_DIR:=$(BASE_DIR)/bin
SRC:=./$(DRIVER_NAME).tmpl
TARGET:=$(BIN_DIR)/$(DRIVER_NAME)
ALLDEP+=./Makefile

all:	$(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC) FORCE | $(BIN_DIR) # just always do this -- depends on ENV vars
	sed -e "s|@CULAM_PATH@|$(ROOT_DIR)/bin/culam|" -e "s|@MFM_ROOT_DIR@|$(MFM_ROOT_DIR)|" $< >$@
	chmod 755 $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

realclean:	clean
	@rm -f ${TARGET}

.PHONY:	FORCE
