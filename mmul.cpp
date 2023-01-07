#include <iostream>
#include "matrix.hpp"
#include <sys/time.h>

using namespace std;

const size_t SIZE = 1024;
typedef Matrix& MatrixRef;

void MultiplyMatrices(const MatrixRef a, const MatrixRef b, MatrixRef c) {
  for (size_t i = 0; i < SIZE; i++) {
    for (size_t j = 0; j < SIZE; j++) {
      for (size_t k = 0; k < SIZE; k++) {
        c[i][j] += a[i][k] * b[j][k];
      }
    }
  }
}

int main() {
  Matrix a(SIZE);
  Matrix b(SIZE);
  Matrix c(SIZE);

  struct timeval start, end;

  gettimeofday(&start, NULL);

  MultiplyMatrices(a, b, c);

  gettimeofday(&end, NULL);

  double elapsedtime_sec = double(end.tv_sec - start.tv_sec) + 
                             double(end.tv_usec - start.tv_usec)/1000000.0;
  cout << "Multiplication time (N=" << SIZE << "): " << elapsedtime_sec << std::endl;

  float checksum = 0.0f;
  for (size_t i = 0; i < SIZE; i++) {
    for (size_t j = 0; j < SIZE; j++) {
      checksum += c[i][j];
    }
  }

  cout << "MMchecksum = " << checksum << endl;

  return 0;
}
