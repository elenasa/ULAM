BASE_DIR:=../..
BUILD_DIR:=$(BASE_DIR)/build/ulam
INCLUDE_DIR:=$(BASE_DIR)/include

INCLUDES += -I${INCLUDE_DIR}

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard $(INCLUDE_DIR)/*.h)
INCS := $(wildcard $(INCLUDE_DIR)/*.inc)

all:	$(BUILD_DIR)/libulam.a

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/libulam.a: ${OBJS}
	ar ruc $@ $^
	ranlib $@

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(INCS) $(ALLDEP) | $(BUILD_DIR)
	g++ -c ${CFLAGS} ${INCLUDES} $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f $(INCLUDE_DIR)/*~
	@rm -f ${OBJS}
	@rm -f $(BUILD_DIR)/libulam.a

.PHONY:	FORCE
