#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3195_test_compiler_element_bitstype_assignarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bits(3) m_i[3](1,2,3);  Bits(3) m_b[3](1,2,3);  Int(32) test() {  m_i 0 [] 1 cast = m_i 1 [] 2 cast = m_i 2 [] m_i 0 [] cast m_i 1 [] cast +b cast = m_b m_i = m_b 2 [] cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // not to mention different types! error message. array cast not implemented. try same type OK.
      bool rtn1 = fms->add("Foo.ulam","ulam 1;  element Foo { Bits(3) m_i[3]; Bits(3) m_b[3]; Int test() { m_i[0] = 1; m_i[1] = 2; m_i[2] = m_i[0] + m_i[1]; m_b = m_i; return m_b[2]; } }\n"); 


      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3195_test_compiler_element_bitstype_assignarray)
  
} //end MFM


