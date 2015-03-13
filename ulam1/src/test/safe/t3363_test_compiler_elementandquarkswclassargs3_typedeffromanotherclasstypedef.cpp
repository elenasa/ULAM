#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3363_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      //Constants have explicit types
      return std::string("Exit status: 3\nUe_R { Int(32) test() {  typedef P(3) Foo;  P(3) pvar;  3u cast return } }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef P(a) Foo;  <NOMAIN> }\nUe_P { constant Int(32) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3341
      // recursive typedefs as typedef
      // must already be parsed!
      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nelement R {\nInt test() {\ntypedef V.Woof.Foo Foo;\n Foo pvar;\n return pvar.sizeof;\n}\n}\n");

      bool rtn1 = fms->add("P.ulam","ulam 1;\nelement P(Int a) {\nBool(a) b;\n}\n");

      // as primitive, as regular class, we have no problems.
      bool rtn2 = fms->add("Q.ulam","ulam 1;\nuse P;\nquark Q(Int s) {\ntypedef P(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3363_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)

} //end MFM
