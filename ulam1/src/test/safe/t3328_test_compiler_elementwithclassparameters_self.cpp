#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3328_test_compiler_elementwithclassparameters_self)
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
      // simple case: OK!  "Bool(x+y) b1, b2;" works!!
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\n Bool(x+y) b1, b2;\n Int test() {\nS(1,2) s;\n/*Atom a;\n a = self;\n self = a;\n*/ return 0;\n }\n }\n");

      //"(self is S(x,y)" doesn't work!
      bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\n Bool func() {\n return (self is S(x,y));\n}\n Int test() {\nS(1,2) s12;\n s12.sp = s12.func();\n return 0;\n }\n }\n");

      // works, BUT "return (self is S(x,y))" doesn't work!
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S(Int x, Int y){\nBool sp;\n Bool(x) b1, b2;\n Bool func() {\n return (self is S(x,y));\n}\n  Bool func(Atom d) {\n return (d is S(x,y));\n}\n Int test() {\nAtom a;\nS(x,y) s12;\n s12.b2 = s12.func();\n s12.b1 = s12.func(a);\n return s12.b2;\n }\n }\n");


      if(rtn1)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3328_test_compiler_elementwithclassparameters_self)

} //end MFM


