#include "matrix.hpp"
#include <cstdlib>

Matrix::Matrix(size_t size, bool invert) {
  SIZE = size;
  //Allocate size rows with size elements each
  theMatrix.resize(SIZE*SIZE);

  //Inversion for random initialization matters if we 
  // want consistent results for a given seed.
  if (!invert) {
    for (size_t i = 0; i < size; i++) {
      for (size_t j = 0; j < size; j++) {
        (*this)[i][j] = float(rand() % 100);
      }
    }
  } else {
    for (size_t i = 0; i < size; i++) {
      for (size_t j = 0; j < size; j++) {
        (*this)[j][i] = float(rand() % 100);
      }
    }
  }
}

Matrix::Row Matrix::operator[](size_t i) {
  return &theMatrix[i*SIZE];
}

Matrix::ConstRow Matrix::operator[](size_t i) const {
  return &theMatrix[i*SIZE];
}
