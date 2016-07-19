#include <iostream>

int main()
  {
    int * a = new int[10];
    int & b = a;
    for(int i = 0; i < 10; i++)
      a[i] = i*2;

    for(int j = 0; j < 10; j++)
      {
	std::cout << j << " : " << (b + j) << " == " << *(a + j) << std::endl;
      }

    return 1;
  }
  
