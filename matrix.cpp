#include "matrix.hpp"
#include <cstdlib>

Matrix::Matrix(size_t size, bool invert) {
  //Allocate size rows with size elements each
  resize(size);
  for (size_t i = 0; i < size; i++)
    (*this)[i].resize(size);

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

Matrix::Row& Matrix::operator[](size_t i) {
  return data()[i];
}

const Matrix::Row& Matrix::operator[](size_t i) const {
  return data()[i];
}
