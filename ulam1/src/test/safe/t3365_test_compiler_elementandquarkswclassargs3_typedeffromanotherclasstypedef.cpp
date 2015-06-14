#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3365_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: -1\nUe_R { Unsigned(32) psize(3);  Int(32) test() {  psize 3u = pvar(-1) cast return } }\nUq_V { typedef Q(3) Woof;  <NOMAIN> }\nUq_Q { constant Int(32) s = NONREADYCONST;  typedef Int(UNKNOWN) Foo;  <NOMAIN> }\nUe_P { constant Int(32) a = NONREADYCONST;  Bool(UNKNOWN) b(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3364
      // recursive typedefs as Element Parameter datamember variable type
      // must already be parsed!
      bool rtn4 = fms->add("R.ulam","ulam 1;\nuse V;\nelement R {\nparameter V.Woof.Foo pvar = -1;\n Unsigned psize;\n Int test() {\n psize = pvar.sizeof;\nreturn pvar;\n}\n}\n");

      bool rtn1 = fms->add("P.ulam","ulam 1;\nelement P(Int a) {\nBool(a) b;\n}\n");

      // as primitive, as regular class, we have no problems.
      bool rtn2 = fms->add("Q.ulam","ulam 1;\nuse P;\nquark Q(Int s) {\ntypedef Int(s) Foo;\n}\n"); //was P(s)

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q(3) Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3365_test_compiler_elementandquarkswclassargs3_typedeffromanotherclasstypedef)

} //end MFM
