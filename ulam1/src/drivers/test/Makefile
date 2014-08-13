BASE_DIR:=../../..
DRIVER_NAME:=test
BUILD_DIR:=$(BASE_DIR)/build/drivers/$(DRIVER_NAME)
BIN_DIR:=$(BASE_DIR)/bin
INCLUDE_DIR:=$(BASE_DIR)/include
ULAM_LIB_DIR:=$(BASE_DIR)/build/ulam
TEST_LIB_DIR:=$(BASE_DIR)/build/test

CFLAGS += -ansi -Wall -pedantic
INCLUDES += -I${INCLUDE_DIR}

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard $(INCLUDE_DIR)/*.h)

all:	$(BIN_DIR)/$(DRIVER_NAME)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/$(DRIVER_NAME): ${OBJS} | $(BIN_DIR)
	g++ $(OBJS) -L $(TEST_LIB_DIR) -ltests -L $(ULAM_LIB_DIR) -lulam  -o $@
##	$(BIN_DIR)/test

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(ALLDEP) | $(BUILD_DIR)
	g++ -c ${CFLAGS} ${INCLUDES} $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

.PHONY:	FORCE
