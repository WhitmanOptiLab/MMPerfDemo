#pragma once 
#include <vector>

//A simple square Matrix class
class Matrix : public std::vector< std::vector<float> > {
 public:
   typedef std::vector<float> Row;
   Matrix(size_t size, bool invert = false);
   Row& operator[](size_t i);
   const Row& operator[](size_t i) const;
};

