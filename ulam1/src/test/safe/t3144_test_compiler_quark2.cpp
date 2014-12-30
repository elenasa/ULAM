#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3144_test_compiler_quark2)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Foo { Int(7) val(0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // notice the filename must match the class name including capitalization.
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n quark Foo {\nInt(7) val;\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3144_test_compiler_quark2)

} //end MFM


