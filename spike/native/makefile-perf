#note: tests run from 'rootdir'/src/test/bin
ULAM_ROOT_DIR:=../../..
-include $(ULAM_ROOT_DIR)/Makefile.local.mk

#MFM_BASE_DIR:=/home/elenas/WORK/ulam/repo/MFMv2
MFM_INCLUDES:=$(MFM_ROOT_DIR)/src/core/include -I $(MFM_ROOT_DIR)/src/sim/include
MFM_PLATFORM_INCL:=$(MFM_ROOT_DIR)/src/platform-linux/include
MFM_BUILD_CORE:=$(MFM_ROOT_DIR)/build/core
MFM_BUILD_PLATFORM:=$(MFM_ROOT_DIR)/build/platform-linux

CFLAGS:= -pthread -ansi -pedantic
CFLAGS+= -Wall
# Optimization-99 instead of gdb flag for performance testing
#CFLAGS+= -g
CFLAGS+= -O99
#CFLAGS+= -Werror
#CFLAGS+= -Wunused-variable
default: all

all:
	g++ -c $(CFLAGS) -D MFM_BUILD_DATE=1 -D MFM_BUILD_TIME=2 -I include -I $(MFM_INCLUDES) -I $(MFM_PLATFORM_INCL) ./src/*.cpp
#	g++ -c $(CFLAGS) -I include -I $(MFM_INCLUDES) -I $(MFM_PLATFORM_INCL) ./src/*.cpp
	g++ -L $(MFM_BUILD_CORE) -L $(MFM_BUILD_PLATFORM) ./*.o -l mfmcore -l mfmplatform-linux -l mfmcore -pthread -o main
#	valgrind ./main

clean: 	FORCE
	rm -f *.o
	rm -f main
	rm -f *~

.PHONY:	FORCE
