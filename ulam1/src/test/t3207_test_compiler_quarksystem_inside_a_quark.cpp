#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3207_test_compiler_quarksystem_inside_a_quark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_System { <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn3)
	return std::string("System.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3207_test_compiler_quarksystem_inside_a_quark)
  
} //end MFM


