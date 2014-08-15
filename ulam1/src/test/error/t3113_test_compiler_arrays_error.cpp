#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3113_test_compiler_arrays_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" a.ulam:1:33: ERROR: Not storeIntoAble: <a>, is type: <Int.32.3>.\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[3]; Int test() { a = 1; return a[1]; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3113_test_compiler_arrays_error)
  
} //end MFM


