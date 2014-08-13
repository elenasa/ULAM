#include <iostream>

int main()
  {
    int a = 1;
    int b = 3;
    int c = a & b;
    std::cout << "1 & 2 = " <<  c << std::endl;


    float f = 2.3;
    float g = 3.1;
    //float h = f & g;

    //    std::cout << "float " << f << " & int " << b  << " = float " <<  g << std::endl;
    //std::cout << "float " << f << " & float " << g  << " = float " <<  h << std::endl;

    bool x = false;
    bool y = false;
    bool z = x & y;

    std::cout << "bool " << x << " & bool " << y << " = " << z << std::endl;

#if 0
    std::cout << "! bool t = " << ! y << std::endl;
    bool z= false;
    std::cout << " bool f = " <<  z << std::endl;
    std::cout << "! bool f = " << ! z << std::endl;

#endif
    return 1;
  }
  
