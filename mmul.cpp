#include <iostream>
#include "matrix.hpp"
#include <sys/time.h>

using namespace std;

const size_t SIZE = 1024;
const size_t TILE = 16;

typedef float float4 __attribute__ ((vector_size(16)));
typedef Matrix& MatrixRef;

void MultiplyMatrices(const MatrixRef a, const MatrixRef b, MatrixRef c) {
#pragma omp parallel for
  for (size_t i_out = 0; i_out < SIZE; i_out+=TILE) {
    for (size_t j_out = 0; j_out < SIZE; j_out+=TILE) {
      for (size_t k_out = 0; k_out < SIZE; k_out+=TILE) {
        for (size_t i = i_out; i < i_out+TILE; i++) {
          for (size_t j = j_out; j < j_out+TILE; j++) {
            float4 sum = {0.0f, 0.0f, 0.0f, 0.0f};
            for (size_t k = k_out; k < k_out+TILE; k+=4) {
              sum += *((float4*)&a[i][k]) * *((float4*)&b[j][k]);
            }
            float vals[4];
            *((float4*)vals) = sum;
            c[i][j] += vals[0] + vals[1] + vals[2] + vals[3];
          }
        }
      }
    }
  }
}

int main() {
  Matrix a(SIZE);
  Matrix b(SIZE, true);
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
