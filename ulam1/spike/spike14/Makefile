BASE_DIR:=../..
CFLAGS += -ansi -Wall -pedantic
HEADERS:= $(BASE_DIR)/include
LIBDIR:= $(BASE_DIR)/build
INCLUDES = -I. -I$(HEADERS)
ULAMLIB = -L$(LIBDIR)/ulam -lulam

BIN_DIR = ./bin
BUILD_DIR = ./build

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
HDRS := $(wildcard $(INCLUDE_DIR)/*.h)
GOALS := $(patsubst %.cpp,$(BIN_DIR)/%,$(SRCS))

#all:	$(GOALS) $(warning s==$(SRCS) foo==$(GOALS))
all:	realclean $(GOALS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%: $(BUILD_DIR)/%.o  | $(BIN_DIR)
	g++ $< $(ULAMLIB) -o $@

$(BUILD_DIR)/%.o: %.cpp $(HDRS) $(ALLDEP) | $(BUILD_DIR)
	g++ -c $(CFLAGS) $(INCLUDES) $< -o $@

clean:	FORCE
	@rm -f *~
	@rm -f $(INCLUDE_DIR)/*~
	@rm -f $(OBJS)


realclean:	clean
	rm -f $(GOALS)

pure:
	make -C test $@

testf:
	make -C test $@

tests:
	make -C test $@

.PHONY:	FORCE