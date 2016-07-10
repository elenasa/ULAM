#include <iostream>

int main()
  {
    int a = 0;
    int b = 2;
    std::cout << "! int 0 = " << ! a << std::endl;
    std::cout << "! int 2 = " << ! b << std::endl;



    float f = 2.3;
    std::cout << "float 2.3 = " <<  f << std::endl;
    std::cout << "! float 2.3 = " << ! f << std::endl;

    float g = 0.0;
    std::cout << "! float 0.0 = " << ! g << std::endl;

    bool x = true;
    bool y = true;

    std::cout << "bool t = " << y << std::endl;
    std::cout << "! bool t = " << ! y << std::endl;
    bool z= false;
    std::cout << " bool f = " <<  z << std::endl;
    std::cout << "! bool f = " << ! z << std::endl;


    return 1;
  }
  
