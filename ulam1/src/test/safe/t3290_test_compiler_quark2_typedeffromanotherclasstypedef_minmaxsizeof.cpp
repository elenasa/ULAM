#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3290_test_compiler_quark2_typedeffromanotherclasstypedef_minmaxsizeof)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_V { typedef Q Woof;  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Q.ulam","ulam 1;\nquark Q {\ntypedef Unsigned(3) Foo;\n}\n");

      bool rtn3 = fms->add("V.ulam","ulam 1;\nuse Q;\n quark V {\ntypedef Q Woof;\n}\n");

      if(rtn2 && rtn3)
	return std::string("V.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3290_test_compiler_quark2_typedeffromanotherclasstypedef_minmaxsizeof)

} //end MFM


