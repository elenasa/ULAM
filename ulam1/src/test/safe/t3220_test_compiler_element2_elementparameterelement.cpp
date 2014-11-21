#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3220_test_compiler_element2_elementparameterelement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Poo { Bool(1) valb[3](false,false,false);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      if(rtn2)
	return std::string("Poo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3220_test_compiler_element2_elementparameterelement)
  
} //end MFM


