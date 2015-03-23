#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3290_test_compiler_elementandquarks_typedeffromanotherclasstypedef_minmaxsizeof)
  {
    std::string GetAnswerKey()
    {
      /* no gen code output */
      // several casts unneeded.
      return std::string("Exit status: 3\nUe_P { Bool(1) b(false);  Unsigned(3) x(0);  Unsigned(3) y(7);  Unsigned(3) z(3);  Unsigned(3) i(0);  Unsigned(3) j(7);  Unsigned(3) k(3);  Int(32) test() {  Unsigned(3) var;  var 3 cast = x 0u = y 7u = z 3u cast = i 0u = j 7u = k 3u cast = var cast return } }\nUq_Q { typedef Unsigned(3) Foo;  <NOMAIN> }\nUq_V { typedef Q Woof;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // recursive typedefs
      // must already be parsed!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P {\nBool b;\nUnsigned(3) x,y,z;\n Unsigned(3) i,j,k;\n Int test() {\nV.Woof.Foo var = 3;\nx = var.minof;\n y = var.maxof;\n z = var.sizeof;\n i = V.Woof.Foo.minof;\n j = V.Woof.Foo.maxof;\n k = V.Woof.Foo.sizeof;\n return var;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q {\ntypedef Unsigned(3) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3290_test_compiler_elementandquarks_typedeffromanotherclasstypedef_minmaxsizeof)

} //end MFM
