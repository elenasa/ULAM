#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3350_test_compiler_elementandquarkswclassargs2_bitsizearg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_R { Int(32) test() {  3u = var const P(3u) pel;  3u cast return } }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUe_P { constant Unsigned(3) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // informed by 3349 (didn't link!): without 'var' as typedeffromanotherclass
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\nInt test() {\nconstant V.Woof.Foo var = 3u;\n P(var) pel;\n return 0;\n}\n}\n");

      // this worked, (didn't link!) needed R helper.
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\nInt test() {\nconstant Unsigned(3) x = 3u; P(x) pel;\n return 0 /*pel.sizeof*/;\n}\n}\n");

      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nuse P;\nelement R {\nInt test() {\nconstant V.Woof.Foo var = 3u;\n P(var) pel;\n return pel.sizeof;\n}\n}\n");

      bool rtn1 = fms->add("P.ulam","ulam 1;\n element P(Unsigned(3) a) {\nBool(a) b;\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3350_test_compiler_elementandquarkswclassargs2_bitsizearg)

} //end MFM
