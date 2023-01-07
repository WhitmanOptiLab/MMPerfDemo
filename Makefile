all: perftest

Makefile:

perftest: mmul
	./mmul

mmul: mmul.o
	g++ $^ -o $@

%.o : %.cpp matrix.hpp Makefile
	g++ -c -o $@ $< -O3 -fno-tree-loop-vectorize

.PHONY: clean
clean:
	rm -f ./*.o mmul
