#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3217_test_compiler_quark_elementparameterquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { Bool(1) valb[3](false,false,false);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3217_test_compiler_quark_elementparameterquark)
  
} //end MFM


