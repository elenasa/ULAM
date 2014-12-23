#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3268_test_compiler_quark_typedeffromanotherclasstypedef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Q { typedef Unsigned(3) Foo;  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q {\ntypedef Unsigned(3) Foo;\n}\n");

      if(rtn2)
	return std::string("Q.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3268_test_compiler_quark_typedeffromanotherclasstypedef)

} //end MFM


