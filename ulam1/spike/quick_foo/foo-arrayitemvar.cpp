#include <iostream>
struct foo
{
  int a[3];
};

int main()
  {
    foo f;
    int i = 0;
    while(i < 4)
      {
	f.a[i+=1] = i;
	std::cout << "a sub " << i << " is " << f.a[i]<< std::endl;
      }
    return 1;
  }
  
