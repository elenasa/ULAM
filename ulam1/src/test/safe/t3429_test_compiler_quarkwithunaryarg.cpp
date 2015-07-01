#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3429_test_compiler_quarkwithunaryarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Bar(3) bar( constant Unary(7) s = 3;  typedef Unsigned(7) US[7];  Bool(1) b(false); );  Int(32) test() {  Bool(1) bs;  bs bar ( )check . = bs cast return } }\nUq_Bar { constant Unary(7) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) US[UNKNOWN];  Bool(1) b(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3405
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\n element Foo {\nBar(3) bar;\n Int test() {\nBool bs = bar.check();\nreturn bs;\n }\n }\n");

      //added typedef to use unary constant as bitsize and arraysize, instead of this error:
      //./Bar.ulam:3:23: ERROR: Array size specifier in [] is not a constant number.
      bool rtn2 = fms->add("Bar.ulam","ulam 1;\n quark Bar(Unary(7) s) {\ntypedef Unsigned(s) US[s];\n Bool b;\n Bool check(){\n b = (s == 7);\n return b;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3429_test_compiler_quarkwithunaryarg)

} //end MFM
