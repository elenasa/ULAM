#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3149_test_compiler_elementandquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_Foo { Int(4) m_i(0);  Bar m_bar( Bool(1) val_b[3](true,false,false); );  Int(32) test() {  Foo f;  m_bar val_b 0 [] . f ( 1 cast )check . = 7 cast return } }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n Int(4) m_i;\n Bar m_bar;\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n m_bar.val_b[0] = f.check(1);\n return 7;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3149_test_compiler_elementandquark)

} //end MFM


