#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3183_test_compiler_element_unarycastconstant)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Unary(4) m_u[5](1,0,3,2,0);  Int(4) m_i(2);  Bool(1) m_b(true);  Int(32) test() {  m_u 0 [] true cast = m_u 1 [] false cast = m_u 2 [] 3 cast = m_u 3 [] 2 cast = m_u 4 [] 1 - cast = m_i m_u 3 [] cast = m_b m_u 3 [] cast = m_u 3 [] cast return } }\nExit status: 2");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Unary(4) m_u[5];  Int(4) m_i;  Bool m_b;  Int test() { m_u[0] = true; m_u[1] = false; m_u[2] = 3; m_u[3] = 2; m_u[4] = -1;  m_i = m_u[3]; m_b = m_u[3];  return m_u[3]; } }\n");

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3183_test_compiler_element_unarycastconstant)
  
} //end MFM


