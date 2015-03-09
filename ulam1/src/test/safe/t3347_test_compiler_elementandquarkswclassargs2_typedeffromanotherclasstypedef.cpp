#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3347_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      //Constants have explicit types
      return std::string("Exit status: 3\nUe_P { constant Unsigned(32) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  Unsigned(UNKNOWN) var(0);  Int(32) test() {  P(1u) pel;  pel var . 3u = pel var . return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // informed by 3339: P is now parametric
      // recursive typedefs as local variable type
      // must already be parsed!

      //this works!!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Unsigned a) {\nBool(a) b;\nV.Woof.Foo var;\n Int test() {\n P(1u) pel;\n pel.var = 3u;\n return pel.var;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3347_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)

} //end MFM
