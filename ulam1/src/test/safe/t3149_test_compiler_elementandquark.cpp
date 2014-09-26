#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3149_test_compiler_elementandquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(32) m_i(0);  Bar m_bar( Bool(1) val_b[3](true,false,false); );  Int(32) test() {  Foo f;  m_bar val_b 0 [] . f ( 1 cast )check . = 7 cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int m_i; Bar m_bar; Bool check(Int v) { return true; } Int test() { Foo f; m_bar.val_b[0] = f.check(1); return 7; } }\n"); 

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3149_test_compiler_elementandquark)
  
} //end MFM


