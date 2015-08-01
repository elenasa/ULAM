#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3355_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_P { constant Int(32) a = NONREADYCONST;  Bool(1) b(false);  Unsigned(UNKNOWN) var(0);  Int(32) test() {  P(1) pel;  pel var . 3 = pel var . cast return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3345: (except with casts added)
      // recursive typedefs as data member variable type of regular class instantiated (pel)
      // must already be parsed!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\nelement P(Int a){\nBool b;\nV.Woof.Foo var;\nInt test() {\n P(1) pel;\n pel.var = 3;\n return (Int) pel.var;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3355_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)

} //end MFM
