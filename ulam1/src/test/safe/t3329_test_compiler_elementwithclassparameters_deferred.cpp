#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3329_test_compiler_elementwithclassparameters_deferred)
  {
    //informed by t3255
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x7 (true)
	 Bool(3) Arg: 0x0 (false)
      */
      //note: in eval, last case, uninitialized atom case is wrong!!
      return std::string("Exit status: 0\nUe_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Bool(1) sp(false);  Bool(3) b1(false);  Bool(3) b2(false);  Int(32) test() {  Atom(96) a;  S s12;  s12 b2 . s12 ( )func . = s12 b1 . s12 ( a )func . = s12 b2 . return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\nInt func(){\n return x + y;\n}\n Int test() {\nS(1,2) s12;\nreturn s12.func();\n }\n }\n");
      bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\nInt func(){\nS(x,y) s; return x + y;\n}\n Int test() {\nS(1,2) s12;\nreturn s12.func();\n }\n }\n");


      if(rtn1)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3329_test_compiler_elementwithclassparameters_deferred)

} //end MFM


