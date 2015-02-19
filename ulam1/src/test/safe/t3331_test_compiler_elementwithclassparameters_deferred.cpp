#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3331_test_compiler_elementwithclassparameters_deferred)
  {
    //informed by t3330
    std::string GetAnswerKey()
    {
      //note: in eval, last case, uninitialized atom case is wrong!!
      return std::string("Exit status: 6\nUe_T { Int(32) test() {  S(1,2) m;  m ( )func . return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can't return named constant in another class 'return S(1,2).x;'
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\n Int test(){\nS(1,2) m;\n return m.func();\n}\n }\n");

      //infinite loop 'S(x+y,n) s;' with x+y as class arg!
      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(){\nreturn S(x,y).sizeof;\n}\n }\n");

      if(rtn1 & rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3331_test_compiler_elementwithclassparameters_deferred)

} //end MFM


