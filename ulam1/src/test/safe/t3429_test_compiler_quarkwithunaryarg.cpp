#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3429_test_compiler_quarkwithunaryarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_Foo { Bar(3) bar( constant Unary(7) s = 3;  typedef Unsigned(3) US[3];  Bool(1) b(true); );  Int(32) test() {  Bool(1) bs;  bs bar ( )check . = bs cast return } }\nUq_Bar { constant Unary(7) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) US[UNKNOWN];  Bool(1) b(true);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3405
      // Bar(8) error:
      //./Foo.ulam:4:5: ERROR: Constant value expression for (s = 8) is not representable as Unary(7).
      // Bar (-1) error:
      //./Foo.ulam:4:5: ERROR: Constant value expression for (s = -1) is not representable as Unary(7).
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\n element Foo {\nBar(3) bar;\n Int test() {\nBool bs = bar.check();\nreturn (Int) bs;\n }\n }\n");

      //added typedef to use unary constant as bitsize and arraysize, instead of this error:
      //./Bar.ulam:3:23: ERROR: Array size specifier in [] is not a constant number.
      bool rtn2 = fms->add("Bar.ulam","ulam 1;\n quark Bar(Unary(7) s) {\ntypedef Unsigned(s) US[s];\n Bool b;\n Bool check(){\n b = (s == 3);\n return b;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3429_test_compiler_quarkwithunaryarg)

} //end MFM
