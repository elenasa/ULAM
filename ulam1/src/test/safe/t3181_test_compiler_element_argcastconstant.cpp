#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3181_test_compiler_element_argcastconstant)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(1);  Bool(1) m_b(true);  Int(32) test() {  m_i true cast = Bool(3) b;  b true cast = m_b ( true cast )check = m_b cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i; Bool m_b; Bool check(Bool(2+1) b) { return b /* true */; } Int test() { m_i = true; Bool(3) b; b = true; m_b = check( true /*b*/); return m_b; } }\n"); //constant Args cast automatically

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3181_test_compiler_element_argcastconstant)
  
} //end MFM


