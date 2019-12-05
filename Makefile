.PHONY: all clean
src=$(wildcard ./*.cpp)
bin=$(src:%.cpp=%)
CC=g++
all:$(bin)
clean:
	rm $(bin)
