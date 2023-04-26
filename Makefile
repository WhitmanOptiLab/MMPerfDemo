all: perftest

Makefile:

perftest: mmul
	./mmul

mmul: mmul.o matrix.o
	g++ $^ -o $@ -lOpenCL

%.o : %.cpp matrix.hpp Makefile
	g++ -c -o $@ $< -fno-tree-loop-vectorize

.PHONY: clean
clean:
	rm -f ./*.o mmul
