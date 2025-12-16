CXX = g++
CXXFLAGS = -std=c++26 -O0 -Wall -Wextra -Werror -pedantic

SRC = $(wildcard *.cpp)
EXE = app

all: clean build

build: main.cpp
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE) && mkdir bin/ && mv $(EXE) bin/

run: all
	./$(EXE)

clean:
	rm -rf bin/
