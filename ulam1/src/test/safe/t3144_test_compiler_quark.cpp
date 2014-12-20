#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3144_test_compiler_quark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { Foo valoo( Int(7) val(0); );  Int(32) test() {  1 cast return } }\nExit status: 1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // notice the filename must match the class name including capitalization.
      bool rtn2 = fms->add("Bar.ulam","ulam 1;\n use foo;\n quark Bar {\n Foo valoo;\n Int test() {\n return 1;\n}\n }\n");
      bool rtn1 = fms->add("foo.ulam","ulam 1;\n quark Foo {\nInt(7) val;\n }\n");

      if(rtn2 && rtn1)
	return std::string("Bar.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3144_test_compiler_quark)

} //end MFM


