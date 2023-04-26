#pragma once 
#include <vector>

//A simple square Matrix class
class Matrix {
 private:
   std::vector< float > theMatrix;
   size_t SIZE;

 public:
   typedef float* Row;
   typedef const float* ConstRow;
   Matrix(size_t size, bool invert = false);
   Row operator[](size_t i);
   ConstRow operator[](size_t i) const;
};

#include "matrix.cpp"
