#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3330_test_compiler_elementwithclassparameters_deferred)
  {
    //informed by t3255
    std::string GetAnswerKey()
    {
      //note: in eval, last case, uninitialized atom case is wrong!!
      return std::string("Exit status: 0\nUe_T { S(1,2) m( constant Int(CONSTANT) x = 1;  constant Int(CONSTANT) y = 2; );  Int(32) test() {  m ( )func . 0 cast return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //infinite loop 'S(x+y,n) s;' with x+y as class arg!
      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nVoid func(){\nconstant Int n = 6;\nS(x+0, n) s;\n return;\n}\n }\n");

      //can't return named constant in another class 'return S(1,2).x;'
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\nS(1,2) m;\n Int test() {\nm.func();\n return 0;\n}\n }\n");

      if(rtn1 & rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3330_test_compiler_elementwithclassparameters_deferred)

} //end MFM


