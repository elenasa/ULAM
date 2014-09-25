#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3193_test_compiler_element_bitstype)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bits(3) m_i[3](1,2,3);  Bool(3) m_b[3](false,false,true);  Int(32) test() {  m_i 0 [] 1 cast = m_i 1 [] 2 cast = m_i 2 [] m_i 0 [] m_i 1 [] | = m_b 0 [] m_i 0 [] cast = m_b 1 [] m_i 1 [] cast = m_b 2 [] m_i 2 [] cast = m_i 2 [] cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;  element Foo { Bits(3) m_i[3]; Bool(3) m_b[3]; Int test() { m_i[0] = 1; m_i[1] = 2; m_i[2] = m_i[0] | m_i[1]; m_b[0] = m_i[0]; m_b[1] = m_i[1]; m_b[2] = m_i [2]; return m_i[2]; } }\n"); 


      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3193_test_compiler_element_bitstype)
  
} //end MFM


