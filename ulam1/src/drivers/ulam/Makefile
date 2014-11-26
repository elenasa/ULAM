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

$(TARGET): $(SRC) $(ALLDEP) | $(BIN_DIR)
	sed -e "s/@CULAM_PATH@/$(CULAM_PATH)/" -e "s/@MFM_ROOT_DIR@/$(@MFM_ROOT_DIR@)/" $< >$@
	chmod 755 $@

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(ALLDEP) | $(BUILD_DIR)
	g++ -c ${CFLAGS} ${INCLUDES} $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

realclean:	clean
	@rm -f ${TARGET}

.PHONY:	FORCE
