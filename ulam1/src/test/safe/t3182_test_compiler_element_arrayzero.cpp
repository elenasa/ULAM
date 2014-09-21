#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3182_test_compiler_element_arrayzero)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i[0]( );  Bool(1) m_b(true);  Int(32) test() {  m_b ( true cast )check = m_b cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i[0]; Bool m_b; Bool check(Bool b) { return b /* true */; } Int test() { m_b = check(true); return m_b; } }\n"); 

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3182_test_compiler_element_arrayzero)
  
} //end MFM


