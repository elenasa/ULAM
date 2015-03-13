#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3362_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      //Constants have explicit types
      return std::string("Exit status: 3\nUe_P { Bool(1) b(false);  Int(32) test() {  Unsigned(3) var;  var 3 cast = var cast return } }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Unsigned(UNKNOWN) Foo;  <NOMAIN> }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3339, 3349
      // recursive typedefs as local variable type
      // must already be parsed!
      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nelement R {\nInt test() {\n V.Woof.Foo pvar;\n return pvar.sizeof;\n}\n}\n");
      //bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nelement R {\nInt test() {\ntypedef V.Woof.Foo Foo;\n Foo pvar;\n return pvar.sizeof;\n}\n}\n");

      bool rtn1 = fms->add("P.ulam","ulam 1;\nelement P(Int a) {\nBool(a) b;\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nuse P;\nquark Q(Int s) {\ntypedef P(s) Foo;\n}\n");

      //bool rtn2 = fms->add("Q.ulam","ulam 1;\nuse P;\nquark Q(Int s) {\ntypedef Unsigned(s) Foo;\n}\n"); //works!!

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3362_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)

} //end MFM
