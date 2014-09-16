#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3152_test_compiler_fortyonebools)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_B { Bool(1) m_bSites[41](false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false);  Int(32) test() {  m_bSites 20 [] true = m_bSites 20 [] cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("B.ulam","ulam 1; element B { Bool m_bSites[41]; Int test() { m_bSites[20] = true; return m_bSites[20];} }");
      
      if(rtn1)
	return std::string("B.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3152_test_compiler_fortyonebools)
  
} //end MFM


