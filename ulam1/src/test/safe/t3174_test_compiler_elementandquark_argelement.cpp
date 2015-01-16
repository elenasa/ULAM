#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3174_test_compiler_elementandquark_argelement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 5\nUe_Foo { Int(4) m_i(0);  Bar m_bar( Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Int(32) i;  Foo f;  f m_i . 5 cast = i ( f )check = i return } }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // return element
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n Int(4) m_i;\n Bar m_bar;\n Int check(Foo f) {\n return f.m_i;\n }\n Int test() {\n Int i;\n Foo f;\n f.m_i = 5;\n i = check(f);\n return i;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3174_test_compiler_elementandquark_argelement)

} //end MFM


