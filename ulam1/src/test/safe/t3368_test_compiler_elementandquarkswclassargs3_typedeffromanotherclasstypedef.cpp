#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3368_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 4\nUe_R { P(3) pel( constant Int(32) a = 3;  Unsigned(4) var(0); );  Int(32) test() {  P(3) pvar;  ( pvar )punc cast return } }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef P(a) Foo;  <NOMAIN> }\nUq_P { constant Int(32) a = NONREADYCONST;  Unsigned(UNKNOWN) var(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3343, 53 : recursive typedefs as function return value; class with args
      // must already be parsed!
      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nelement R {\nV.Woof.Foo pel;\nUnsigned punc(V.Woof.Foo farg) {\n return farg.sizeof;\n}\n Int test() {\nV.Woof.Foo pvar;\n return (Int) punc(pvar);\n}\n}\n");

      bool rtn1 = fms->add("P.ulam","ulam 1;\nquark P(Int a) {\nUnsigned(a+1) var;\nUnsigned func() {\n return var.sizeof;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nuse P;\nquark Q(Int s) {\ntypedef P(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3368_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)

} //end MFM
