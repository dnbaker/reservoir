
CXX?=g++
INCLUDE=-I. -Iinclude
STD=-std=c++17
FLAGS=-O3 -Wall -Wextra $(INCLUDE) $(STD) -Werror


all: pri rsv crsv parallel_sampling
%: test/%.cpp
	$(CXX) $(FLAGS) $< -o $@ && ./$@
parallel_sampling: test/parallel_sampling.cpp
	$(CXX) $(FLAGS) $< -o $@


clean:
	rm -f pri rsv
