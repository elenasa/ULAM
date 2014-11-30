BASE_DIR:=../..
BUILD_DIR:=$(BASE_DIR)/build/test
INCLUDE_DIR:=$(BASE_DIR)/include
ULAM_LIB_DIR:=$(BASE_DIR)/build/ulam
TGEN_DIR:=./bin

INCLUDES += -I${INCLUDE_DIR}

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard $(INCLUDE_DIR)/*.h)
TSRC := $(wildcard t*.cpp)
TESTS := $(patsubst %.cpp,\nXX(%),$(TSRC))

all:	clean $(BUILD_DIR)/libtests.a

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(TGEN_DIR)

$(BUILD_DIR)/libtests.a: $(OBJS)
	ar ruc $@ $^
	ranlib $@
	@echo '$(TESTS)' > $(BUILD_DIR)/tests.inc

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(ALLDEP) | $(BUILD_DIR)
	g++ -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f $(OBJS)
	@rm -f $(BUILD_DIR)/libtests.a
	@rm -f $(TGEN_DIR)/*.*

.PHONY:	FORCE
