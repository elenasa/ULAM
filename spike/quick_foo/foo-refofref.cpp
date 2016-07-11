#include <iostream>

int main()
  {
    //    int a;
    //typedef int & ir;
    //ir & b = a;
    //int & c = a;

    typedef int BigSite[10]; //try packed first!
    typedef BigSite & BSRef;
    typedef BSRef & BRR;


    return 1;
  }
