CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS := --static -lboost_filesystem -lboost_program_options -lboost_chrono \
	    -lboost_regex -lboost_thread -lboost_system -lcryptopp -lrt -lpthread \
	    libs/libexecstream/exec-stream.o
CC_FLAGS := -DFORK_CAPABLE -DBOOST_CPP0X_PROBLEM -O3 -std=c++0x -I include/ -I libs/libexecstream/

stutsk: $(OBJ_FILES)
	g++ $(OBJ_FILES) ${LD_FLAGS} -o stutsk

obj/%.o: src/%.cpp
	g++ ${CC_FLAGS} -c -o $@ $<

clean:
	rm -f stutsk
	rm -f ./obj/*

