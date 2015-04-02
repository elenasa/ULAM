#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3349_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_R { Int(32) test() {  3u = var const P(3u) pel;  3u cast return } }\nUe_P { constant Unsigned(UNKNOWN) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  <NOMAIN> }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(3) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // LINKER ERRORS if test is in P and not R.
      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse P;\nelement R {\nInt test() {\nconstant V.Woof.Foo var = 3u;\n P(var) pel;\n return pel.sizeof;\n}\n}\n");

      // informed by 3339: P is now parametric
      // recursive typedefs as local variable type
      // must already be parsed!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\n}\n");

      // LINKER ERRORS if test is in P and not R.
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\nInt test() {\nconstant V.Woof.Foo var = 3u;\n P(var) pel;\n return 0;\n}\n}\n");
      //even though this works! (t3350)
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\nInt test() {\nconstant Unsigned(3) x = 3u; P(x) pel;\n return pel.sizeof;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3349_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)

} //end MFM
