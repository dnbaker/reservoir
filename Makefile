.PHONY=all clean


CXX?=g++
INCLUDE=-I. -Iinclude
STD=-std=c++17
FLAGS=-O3 -Wall -Wextra $(INCLUDE) $(STD) -Werror

EX=pri rsv crsv parallel_sampling
all: $(EX)
%: test/%.cpp
	$(CXX) $(FLAGS) $< -o $@ && ./$@


clean:
	rm -f $(EX)
