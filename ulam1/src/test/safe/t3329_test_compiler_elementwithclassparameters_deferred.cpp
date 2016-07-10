#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3329_test_compiler_elementwithclassparameters_deferred)
  {
    //informed by t3255
    std::string GetAnswerKey()
    {
      //Constants have explicit types
      return std::string("Exit status: 3\nUe_S { constant Int(32) x = NONREADYCONST;  constant Int(32) y = NONREADYCONST;  Bool(1) sp(false);  Int(32) test() {  S(1,2) s12;  s12 ( )func . return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\nInt func(){\nS(x,y) s; return x + y;\n}\n Int test() {\nS(1,2) s12;\nreturn s12.func();\n }\n }\n");

      if(rtn1)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3329_test_compiler_elementwithclassparameters_deferred)

} //end MFM
