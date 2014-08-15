#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3112_test_compiler_funcdecl)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Void m(Int m, Bool b[3]); }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //a.ulam:1:32: ERROR: Unexpected token <TOK_SEMICOLON>; Expected <{>.
      //a.ulam:1:8: ERROR: INCOMPLETE Function Definition.
      bool rtn1 = fms->add("a.ulam","ulam { Void m(Int m, Bool b[3]) ; }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3112_test_compiler_funcdecl)
  
} //end MFM


