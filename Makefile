#
# Makefile for stutsk
#

LIB_SRCS = libs/libexecstream/exec-stream.cpp
LIB_OBJS = $(LIB_SRCS:.cpp=.o)

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

CC = g++

CXXFLAGS = -DFORK_CAPABLE -DBOOST_CPP0X_PROBLEM -O2 -std=c++0x -I include/ -I libs/libexecstream/
LDFLAGS  = -lcryptopp -lrt -lpthread
LDFLAGS += -lboost_regex -lboost_thread -lboost_program_options
LDFLAGS += -lboost_filesystem -lboost_chrono -lboost_system

all: libs stutsk

libs: $(LIB_OBJS)

stutsk: $(OBJS)
	$(CC) $(OBJS) $(LIB_OBJS) $(LDFLAGS) -o stutsk-safe

.cpp.o:
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(LIB_OBJS) $(OBJS) stutsk

.PHONY: all libs clean
