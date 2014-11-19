#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3172_test_compiler_quark_returnquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n\n");
      
      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3172_test_compiler_quark_returnquark)
  
} //end MFM


