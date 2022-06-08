BASE_DIR:=../..
BUILD_DIR:=$(BASE_DIR)/build/test
INCLUDE_DIR:=$(BASE_DIR)/include
TGEN_DIR:=$(BASE_DIR)/src/test/bin
SPIKE_DIR:=$(BASE_DIR)/spike
SHARE_DIR:=$(BASE_DIR)/share
MFM_INCLUDE_DIR:=$(MFM_ROOT_DIR)/src/core/include
MFM_PLATFORM_INCLUDE_DIR:=$(MFM_ROOT_DIR)/src/platform-linux/include

INCLUDES += -I${INCLUDE_DIR} -I${MFM_INCLUDE_DIR} -I${MFM_PLATFORM_INCLUDE_DIR}

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
	$(CXX) -c $(CFLAGS) ${DEFINES} $(INCLUDES) $< -o $@

gen: $(TGEN_DIR)
#	cp $(SHARE_DIR)/tcc/*.tcc $(TGEN_DIR)/include/.
	cp $(SPIKE_DIR)/native/*.tcc $(TGEN_DIR)/include/.
	cp $(SPIKE_DIR)/native/makefile $(TGEN_DIR)/.
	make -C $(TGEN_DIR)

clean:	FORCE
	@rm -f *~
	@rm -f $(OBJS)
	@rm -f $(BUILD_DIR)/*.o
	@rm -f $(BUILD_DIR)/libtests.a
	@rm -f $(BUILD_DIR)/tests.inc
	@rm -f $(TGEN_DIR)/src/*.*
	@rm -f $(TGEN_DIR)/include/*.*
	@rm -f $(TGEN_DIR)/*.o
	@rm -f $(TGEN_DIR)/main

.PHONY:	FORCE
