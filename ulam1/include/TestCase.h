/* -*- c++ -*- */

#ifndef TESTCASE_H
#define TESTCASE_H

#include "File.h"

#define BEGINTESTCASE(n)			\
  struct TestCase_##n : public TestCase
  
#define ENDTESTCASE(n) ; static TestCase_##n the_instance;	\
  TestCase * n = &the_instance;


namespace MFM{

  class TestCase
  {
  public:

    virtual ~TestCase() {}

    virtual bool RunTests(File * errorOutput) = 0;

  private:
    
  };

}

#endif //end TESTCASE_H
