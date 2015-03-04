#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3342_test_compiler_elementandquarkswclassargs_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_P { Bool(1) b(false);  Int(32) test() {  3 = nvar const 3u cast return } }\nUq_Q { constant Int(CONSTANT) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3339
      // recursive typedefs
      // must already be parsed!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P {\nBool b;\nInt test() {\nconstant V.Woof.Foo nvar = 3;\n return nvar;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3342_test_compiler_elementandquarkswclassargs_typedeffromanotherclasstypedef)

} //end MFM
