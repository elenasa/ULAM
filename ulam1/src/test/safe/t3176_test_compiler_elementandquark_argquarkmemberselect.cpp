#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3176_test_compiler_elementandquark_argquarkmemberselect)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(1);  Bar m_bar( Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f m_bar ( )reset . . m_i ( f m_bar . )check cast = m_i cast return } }\nExit status: 1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n Int(4) m_i;\n Bar m_bar;\n Bool check(Bar b) {\n return b.val_b[1] /* true */;\n }\n Int test() {\n Foo f;\n   f.m_bar.reset();\n   m_i = check(f.m_bar);\n return m_i;\n }\n }\n"); //worked without f.m_bar as arg to check func; m_i not true? is m_bar f's? YES..now it works.

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset() {\n val_b[1] = true;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3176_test_compiler_elementandquark_argquarkmemberselect)

} //end MFM


