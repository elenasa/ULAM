#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3268_test_compiler_elementandquarks_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_P { Bool(1) b(false);  Int(32) test() {  Unsigned(3) var;  var 3 cast = var cast return } }\nExit status: 3");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // recursive typedefs
      // must already be parsed!
      bool rtn1 = fms->add("P.ulam","ulam 1;\nuse Q;\nuse V;\n element P {\nBool b;\nInt test() {\nV.Woof.Foo var = 3;\n return var;\n}\n}\n");

      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q {\ntypedef Unsigned(3) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q Woof;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("P.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3268_test_compiler_elementandquarks_typedeffromanotherclasstypedef)

} //end MFM


