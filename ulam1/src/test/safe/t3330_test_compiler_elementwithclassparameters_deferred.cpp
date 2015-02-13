#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3330_test_compiler_elementwithclassparameters_deferred)
  {
    //informed by t3255
    std::string GetAnswerKey()
    {
      //note: in eval, last case, uninitialized atom case is wrong!!
      return std::string("Exit status: 0\nUe_T { S m( constant Int(CONSTANT) x = 3;  constant Int(CONSTANT) y = 2; );  Int(32) test() {  m ( )func . 0 cast return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nVoid func(){\nconstant Int n = 6;\nS(x + y, n) s;\n return;\n}\n }\n");

      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\n element T{\nS(1,2) m;\n Int test() {\nm.func();\n return 0;\n}\n }\n");

      if(rtn1 & rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3330_test_compiler_elementwithclassparameters_deferred)

} //end MFM


