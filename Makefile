
CXX?=g++
INCLUDE=-I. -Iinclude
STD=-std=c++17
FLAGS=-O3 -Wall -Wextra $(INCLUDE) $(STD) -Werror


all: pri rsv
%: test/%.cpp
	$(CXX) $(FLAGS) $< -o $@ && ./$@

clean:
	rm -f pri rsv
