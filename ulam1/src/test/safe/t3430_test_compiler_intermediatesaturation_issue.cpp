#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3430_test_compiler_intermediatesaturation_issue)
  {
    std::string GetAnswerKey()
    {
      //correctly g and f are different!
      return std::string("Exit status: 0\nUe_A { typedef Int(9) Foo;  Int(9) f(255);  Int(9) g(254);  Int(32) test() {  g f 255 = = f f cast 1 cast +b cast 1 cast -b cast = g 1 cast += g 1 cast -= f g == cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //aren't the same, but should be???
      //bool rtn1 = fms->add("A.ulam"," element A {\ntypedef Int(9) Foo;\n Foo f;\n Int test(){\n f = f.maxof;\n f += 1;\n f-= 1;\n return f == f.maxof;\n }\n }\n"); //254

      //bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int(9) Foo;\n Foo f;\n Int test(){\nf = f.maxof;\n f = f + 1 - 1;\n return f == f.maxof;\n }\n }\n"); //255

      // in fact, add anything>0 and subtract one gives the same answer.. so maxof == maxof +(1-1) == maxof +(x -1) == maxof +x != maxof +x -(x+1).
      //bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int(9) Foo;\n Foo f,g;\n Int test(){\ng = f = f.maxof;\n f = f + 19 - 1;\n g = g + 1 - 1;\n return f == g;\n }\n }\n");

      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int(9) Foo;\n Foo f,g;\n Int test(){\ng = f = f.maxof;\n f = (Foo) (f + 1 - 1);\n g += 1;\n g -= 1;\n return f == g;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3430_test_compiler_intermediatesaturation_issue)

} //end MFM
