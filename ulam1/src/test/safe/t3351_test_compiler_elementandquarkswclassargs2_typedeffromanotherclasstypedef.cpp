#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3351_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_P { typedef Unsigned(UNKNOWN) Foo;  constant Int(32) a = NONREADYCONST;  Bool(1) b(false);  Unsigned(UNKNOWN) var(0);  Int(32) test() {  P(1) pel;  pel var . 2 = pel var . return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3341: recursive typedefs as typedef type
      // must already be parsed!
      // can an instantiated template use its template' typedef from another class? yes.
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Int a) {\nBool b;\ntypedef V.Woof.Foo Foo;\nFoo var;\n Int test() {\nP(1) pel;\npel.var = 2;\n return pel.var;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3351_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)

} //end MFM
