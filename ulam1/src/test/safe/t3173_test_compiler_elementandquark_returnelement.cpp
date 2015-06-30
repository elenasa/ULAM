#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3173_test_compiler_elementandquark_returnelement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_Foo { Int(4) m_i(0);  Bar m_bar( Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f f ( 7 )check . = f m_i . cast return } }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // return element
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n Int(4) m_i;\n Bar m_bar;\n Foo check(Int i) {\n Foo f;\n f.m_i = (Int(4)) i;\n return f;\n }\n Int test() {\n Foo f;\n f = f.check(7);\n return f.m_i;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = false;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3173_test_compiler_elementandquark_returnelement)

} //end MFM
