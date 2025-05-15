#include "lzip/lzip.hpp"
#include <iostream>

int main(){
  int result = lzip::add_one(1);
  std::cout << "1 + 1 = " << result << std::endl;
}