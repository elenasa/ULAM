#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3175_test_compiler_elementandquark_argquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(1);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  Foo f;  m_bar ( )reset . m_i f ( m_bar )check . cast = f m_i . cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Bool check(Bar b) { return b.val_b[1] /* true */; } Int test() { Foo f;   m_bar.reset();   m_i = f.check(m_bar); return f.m_i; } }\n"); //worked without f.m_bar as arg to check func; m_i not true? is m_bar f's? YES..now it works.

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset() { val_b[1] = true; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3175_test_compiler_elementandquark_argquark)
  
} //end MFM


