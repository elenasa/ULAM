#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3145_test_compiler_quark_posoffset)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { Bool(1) valb[3](false,false,false);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool valb[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3145_test_compiler_quark_posoffset)
  
} //end MFM


