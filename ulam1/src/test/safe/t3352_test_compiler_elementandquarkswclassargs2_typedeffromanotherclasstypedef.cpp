#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3352_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_P { constant Int(32) a = NONREADYCONST;  constant Unsigned(UNKNOWN) nvar = 3;  Int(32) test() {  P(1) pel;  pel nvar . return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3342:  recursive typedefs as named constant type
      // must already be parsed!

      // this works, but limited to 32?
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Int a) {\nconstant V.Woof.Foo nvar = 3u;\n Bool(nvar) b;\nInt test() {\n P(1) pel;\npel.b = true;\nreturn pel.b.sizeof;\n}\n}\n");

      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P{\nconstant V.Woof.Foo nvar = 3u;\nInt test() {\n P pel;\nreturn pel.nvar;\n}\n}\n");

      // ok
      //bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P{\nconstant V.Woof.Foo nvar = 3u;\nInt test() {\nreturn nvar;\n}\n}\n");

      // requires exchangeKids!!
      // return pel.nvar; ---> "ERROR: <nvar> is not a variable and cannot be used as one with class: P(1)"
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P(Int a) {\nconstant V.Woof.Foo nvar = 3u;\nInt test() {\n P(1) pel;\nreturn pel.nvar;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3352_test_compiler_elementandquarkswclassargs2_typedeffromanotherclasstypedef)

} //end MFM
