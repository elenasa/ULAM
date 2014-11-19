#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3152_test_compiler_fortyonebools)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_B { Bool(1) m_bSites[41](false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false);  Int(32) test() {  m_bSites 20 [] true cast = m_bSites 20 [] cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("B.ulam","ulam 1;\n\n element B {\n Bool m_bSites[41];\n Int test() {\n m_bSites[20] = true;\n return m_bSites[20];\n}\n }\n");
      
      if(rtn1)
	return std::string("B.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3152_test_compiler_fortyonebools)
  
} //end MFM


