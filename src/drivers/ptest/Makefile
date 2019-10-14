BASE_DIR:=../../..
DRIVER_NAME:=culamtestperf
BUILD_DIR:=$(BASE_DIR)/build/drivers/$(DRIVER_NAME)
BIN_DIR:=$(BASE_DIR)/bin
INCLUDE_DIR:=$(BASE_DIR)/include

INCLUDES += -I${INCLUDE_DIR}
LIBS := -L $(BASE_DIR)/build/test -ltests $(LIBS)

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard $(INCLUDE_DIR)/*.h)

ALLDEP += $(BASE_DIR)/src/test/*

all:	$(BIN_DIR)/$(DRIVER_NAME)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/$(DRIVER_NAME): ${OBJS} | $(BIN_DIR)
	$(CXX) $(OBJS) $(LIBS) -o $@

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(ALLDEP) | $(BUILD_DIR)
	$(CXX) -c ${CFLAGS} ${DEFINES} ${INCLUDES} $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

.PHONY:	FORCE
