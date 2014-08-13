#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3119_test_compiler_funccall_undefined_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" a.ulam:1:25: ERROR: (2) <fact> is not a defined function, and cannot be called.\na.ulam:1:1: fyi: 1 TOO MANY TYPELABEL ERRORS.\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int main() { a = fact(3); } Int a;  }");  // 6
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3119_test_compiler_funccall_undefined_error)
  
} //end MFM


