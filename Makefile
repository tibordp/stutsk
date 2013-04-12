CPP_FILES := $(wildcard src/*.cpp)
LIBRARY_DEPS := libs/libexecstream/exec-stream.o
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o))) $(LIBRARY_DEPS)
LD_FLAGS := -static -lboost_filesystem -lboost_program_options -lboost_chrono \
	-lboost_regex -lboost_thread -lboost_system -lcryptopp -lrt -lpthread
CC_FLAGS := -DFORK_CAPABLE -DBOOST_CPP0X_PROBLEM -O3 -std=c++0x -I include/ -I libs/libexecstream/

stutsk: $(OBJ_FILES)
	g++ $(OBJ_FILES) ${LD_FLAGS} -o stutsk

libs/libexecstream/exec-stream.o:
	g++ -c libs/libexecstream/exec-stream.cpp -o $@

obj/%.o: src/%.cpp
	g++ ${CC_FLAGS} -c -o $@ $<

clean:
	rm -f stutsk
	rm -f ./obj/*

