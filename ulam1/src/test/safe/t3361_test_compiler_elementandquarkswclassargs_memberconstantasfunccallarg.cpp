#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3361_test_compiler_elementandquarkswclassargs_memberconstantasfunccallarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: -2\nUe_Foo { constant Int(32) a = NONREADYCONST;  Int(32) test() {  Foo(0) f;  f ( )copy . return } }\nUq_Booly { constant Unsigned(UNKNOWN) firstRange = NONREADYCONST;  constant Unsigned(UNKNOWN) lastRange = NONREADYCONST;  constant Bool(1) bomb = NONREADYCONST;  constant Int(32) cALL_FIX2 = -2;  constant Int(32) cALL_FIX4 = -4;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by issue (see 3359): required 3 conditions; caused by missing ClassContext pop after Constant surgery.
      // 1. func call (behave) has at least 2 MEMBER-SELECTED constant args,
      // 2. func call (behave) is also member-selected, and in a member-function (i.e. copy) of Foo (not test);
      // 3. Foo is parametric (not regular).
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Booly;\nelement Foo(Int a){\nInt copy(){\nBooly(0u, 2u, true) b;\n return b.behave(b.cALL_FIX2, b.cALL_FIX4);\n}\n Int test(){\nFoo(0) f; return f.copy();\n}\n}\n");

      bool rtn2 = fms->add("Booly.ulam","quark Booly(Unsigned(3) firstRange, Unsigned(3) lastRange, Bool bomb) {\nconstant Int cALL_FIX2 = -2;\nconstant Int cALL_FIX4 = -4;\nInt behave(Int x, Int y){\n if(bomb && lastRange != firstRange) return x;\nreturn y;\n}\n}\n");


      if(rtn1 && rtn2 )
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3361_test_compiler_elementandquarkswclassargs_memberconstantasfunccallarg)

} //end MFM
