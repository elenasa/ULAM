#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3206_test_compiler_quark_systemnativefuncs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_System { <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn3)
	return std::string("System.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3206_test_compiler_quark_systemnativefuncs)
  
} //end MFM


