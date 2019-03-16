CC=g++
CCFLAGS=-std=c++11

generate: generate.cpp
	$(CC) generate.cpp $(CCFLAGS) -o generate
