#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3205_test_compiler_quark_emptyquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Empty { /* empty class block */ }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {   
      // No Parse Tree
      bool rtn2 = fms->add("Empty.ulam","ulam 1;\nquark Empty {\n /* empty quark comment */ }\n");
      //bool rtn2 = fms->add("Empty.ulam","ulam 1;\nuse System\n; quark Empty {\nSystem s;\n /* empty quark comment */ }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     


      if(rtn2 && rtn3)
	return std::string("Empty.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3205_test_compiler_quark_emptyquark)
  
} //end MFM


