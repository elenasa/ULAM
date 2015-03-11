#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3350_test_compiler_elementandquarkswclassargs2_bitsizearg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_P { constant Unsigned(3) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  Int(32) test() {  3u = x const P(3u) pel;  pel.sizeof cast return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // informed by 3349 (didn't link!): without 'var' as typedeffromanotherclass
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\nInt test() {\nconstant V.Woof.Foo var = 3u;\n P(var) pel;\n return 0;\n}\n}\n");

      // this works!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned(3) a) {\nBool(a) b;\nInt test() {\nconstant Unsigned(3) x = 3u; P(x) pel;\n return pel.sizeof;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3350_test_compiler_elementandquarkswclassargs2_bitsizearg)

} //end MFM
