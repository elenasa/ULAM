#include <iostream>

int main()
  {
    int a = 1;
    int b = 2;
    std::cout << "int + int = " << a + b << std::endl;

    float f = 2.3;
    std::cout << "int + float = " << a + f << std::endl;

    float g = 1.2;
    std::cout << "float + float = " << f + g << std::endl;

    bool x = true;
    bool y = true;

    std::cout << "bool + bool = " << x + y << std::endl;

    std::cout << "+ bool t = " << + y << std::endl;
    bool z= false;
    std::cout << "+ bool f = " << + z << std::endl;


    return 1;
  }
  
