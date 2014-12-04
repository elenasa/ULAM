#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3221_test_compiler_quark_elementwithquarkEP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool val_b[3];\nVoid reset(Bool b) {\nb = 0; }\nInt toInt(){\nreturn 3;}\n}\n");
      
      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3221_test_compiler_quark_elementwithquarkEP)
  
} //end MFM


