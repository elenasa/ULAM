MFM_BASE_DIR:=/home/elenas/WORK/ulam/repo/MFMv2
MFM_INCLUDES:=$(MFM_BASE_DIR)/src/core/include
MFM_PLATFORM_INCL:=$(MFM_BASE_DIR)/src/platform-linux/include
MFM_BUILD_CORE:=$(MFM_BASE_DIR)/build/core
MFM_BUILD_PLATFORM:=$(MFM_BASE_DIR)/build/platform-linux

CFLAGS:= -g -pthread -Wall -ansi -pedantic -Werror
default: all

all:	FORCE
	g++ -c $(CFLAGS) -I ../../../include -I $(MFM_INCLUDES) *.cpp
	g++ -L $(MFM_BUILD_CORE) -L $(MFM_BUILD_PLATFORM) ./*.o -l mfmcore -l mfmplatform-linux -l mfmcore -pthread -o main

clean:
	rm -f *.o
	rm -f main
	rm -f *~

.PHONY:	FORCE

