class foo {

  //g++ -ansi -pedantic -c foo-classfuncswrefs.cpp

public:

  bool m(bool b)
  {
    return true;
  }

  bool m(bool& b)
  {
    return true;  //overloaded ref arg
  }


private:

};
