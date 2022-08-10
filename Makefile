
CPPFLAGS+=-std=c++17 -W -Wall -g -Wno-unused-parameter
CPPFLAGS+=-I include

OBJECTS += src/main.o src/compiler_flexer.yy.o src/compiler_parser.tab.o

all: bin/c_compiler

src/compiler_parser.tab.cpp src/compiler_parser.tab.hpp:: src/parser.y
	bison -v -d src/parser.y -o src/compiler_parser.tab.cpp 


src/compiler_flexer.yy.cpp: src/lexer.flex src/compiler_parser.tab.hpp
	flex -o src/compiler_flexer.yy.cpp src/lexer.flex


bin/c_compiler: src/compiler_parser.tab.o src/compiler_flexer.yy.o  src/main.o
	mkdir -p bin
	mkdir -p out
	g++ $(CPPFLAGS) $^ -o bin/c_compiler

clean :
	rm src/*.o 
	rm bin/* 
	rm src/*.tab.cpp 
	rm src/*.yy.cpp 
	rm src/*.tab.hpp 
	rm src/*.output
	rm out/*