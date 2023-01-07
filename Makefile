all: perftest

perftest: mmul
	./mmul

mmul: mmul.o matrix.o
	g++ $^ -o $@

%.o: %.cpp matrix.hpp mmul.hpp
	g++ -c $< -o $@

