#note: tests run from 'rootdir'/src/test/bin
#note: this makefile copied from 'rootdir'/spike/native
ULAM_ROOT_DIR:=../../..
-include $(ULAM_ROOT_DIR)/Makefile.local.mk

MFM_INCLUDES:=$(MFM_ROOT_DIR)/src/core/include -I $(MFM_ROOT_DIR)/src/sim/include
MFM_PLATFORM_INCL:=$(MFM_ROOT_DIR)/src/platform-linux/include
MFM_BUILD_CORE:=$(MFM_ROOT_DIR)/build/core
MFM_BUILD_SIM:=$(MFM_ROOT_DIR)/build/sim
MFM_BUILD_PLATFORM:=$(MFM_ROOT_DIR)/build/platform-linux

CFLAGS:= -g
## test w optimization instead of -g
#CFLAGS+= -O99
CFLAGS+:= -pthread -ansi -pedantic
CFLAGS+= -Wall
#CFLAGS+= -Werror
CFLAGS+= -Wno-unused-variable -Wno-unused-but-set-variable
#CFLAGS+= -Winline
CFLAGS+= --param inline-unit-growth=1000 --param large-function-growth=50000
CFLAGS+= -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers
CFLAGS+= -Wno-parentheses
#CFLAGS+= -std=c++11

default: all

all:
	$(CXX) -c $(CFLAGS) -D MFM_BUILD_DATE=1 -D MFM_BUILD_TIME=2 -I include -I $(MFM_INCLUDES) -I $(MFM_PLATFORM_INCL) ./src/*.cpp
# mfmcore intentionally linked twice
	$(CXX) -L $(MFM_BUILD_CORE) -L $(MFM_BUILD_SIM) -L $(MFM_BUILD_PLATFORM) ./*.o -l mfmcore -l mfmsim -l mfmplatform-linux -l mfmcore -ldl -pthread -o main
#	valgrind ./main

clean: 	FORCE
	rm -f *.o
	rm -f main
	rm -f *~

.PHONY:	FORCE
