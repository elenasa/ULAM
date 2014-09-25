#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3192_test_compiler_element_unsignedtype)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Unsigned(1) m_array[10](0,1,0,0,0,0,0,0,0,0);  Int(32) test() {  m_array 1 [] 1 cast = m_array 1 [] cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Unsigned(1) m_array[10]; Int test() { m_array[1] = 1; return m_array[1]; } }\n"); 


      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3192_test_compiler_element_unsignedtype)
  
} //end MFM


