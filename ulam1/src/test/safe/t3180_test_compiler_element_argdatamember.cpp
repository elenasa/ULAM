#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3180_test_compiler_element_argdatamember)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(0);  Bool(1) m_b(false);  Int(32) test() {  Foo f;  f m_b . true = m_b f ( m_b )check . = f m_i . cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i; Bool m_b; Bool check(Bool b) { if(b) m_i = 4; else m_i = 3; return b; } Int test() { Foo f; f.m_b = true; m_b = f.check(m_b); return f.m_i; } }\n"); 

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3180_test_compiler_element_argdatamember)
  
} //end MFM


