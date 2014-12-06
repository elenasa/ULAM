BASE_DIR:=../../..
DRIVER_NAME:=eulam
BUILD_DIR:=$(BASE_DIR)/build/drivers/$(DRIVER_NAME)
BIN_DIR:=$(BASE_DIR)/bin
INCLUDE_DIR:=$(BASE_DIR)/include

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
	g++ $(OBJS) $(LIBS) -o $@

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(ALLDEP) | $(BUILD_DIR)
	g++ -c ${CFLAGS} ${INCLUDES} $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

.PHONY:	FORCE
