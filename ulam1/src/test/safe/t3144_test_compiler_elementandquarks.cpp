#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3144_test_compiler_elementandquarks)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_Test { Bar bar( Foo valoo( Int(7) val(1); ); );  Int(32) test() {  bar valoo val . . 1 cast = bar valoo val . . cast return } }\nUq_Bar { Foo valoo( Int(7) val(1); );  <NOMAIN> }\nUq_Foo { Int(7) val(1);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // notice the filename must match the class name including capitalization.
      bool rtn3 = fms->add("Test.ulam","ulam 1;\n use Bar;\nelement Test {\nBar bar;\n Int test() {\nbar.valoo.val = 1; return bar.valoo.val;\n}\n }\n");

      bool rtn2 = fms->add("Bar.ulam","ulam 1;\n use Foo;\n quark Bar {\n Foo valoo;\n }\n");
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n quark Foo {\nInt(7) val;\n }\n");

      if(rtn2 && rtn1 && rtn3)
	return std::string("Test.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3144_test_compiler_elementandquarks)

} //end MFM


