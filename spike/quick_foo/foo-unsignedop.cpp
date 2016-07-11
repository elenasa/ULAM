#include <iostream>

int main()
  {
    int i = -1;
    int j = -2;
    unsigned int a = i;
    unsigned int b = j;
    int c = (int) a;
    int d = (int) b;

    int q = (i)/j;
    unsigned p = (a)/b;
    int r = c/d;
    unsigned s = c/d;

    std::cout << "-int / -int = " << q << ", unsigned == " << p << ", casted == " << r << ", casted unsigned ==" << s << std::endl;

    q = (i)*j;
    p = (a)*b;
    r = c*d;
    s = c*d;

    std::cout << "-int * -int = " << q << ", unsigned == " << p << ", casted == " << r << ", casted unsigned ==" << s << std::endl;


    q = (i)+j;
    p = (a)+b;
    r = c+d;
    s = c+d;

    std::cout << "-int + -int = " << q << ", unsigned == " << p << ", casted == " << r << ", casted unsigned ==" << s << std::endl;



    q = (i)-j;
    p = (a)-b;
    r = c-d;
    s = c-d;

    std::cout << "-int - -int = " << q << ", unsigned == " << p << ", casted == " << r << ", casted unsigned ==" << s << std::endl;




    return 1;
  }
  
