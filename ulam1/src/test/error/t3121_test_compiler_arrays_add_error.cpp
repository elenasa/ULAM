#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3121_test_compiler_arrays_add_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" a.ulam:1:35: ERROR: Incompatible (nonscalar) types, LHS: <Int.32.3>, RHS: <Int.32.0> for binary operator+.\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[3], b; Int test() {return (a + b);} }");
            
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3121_test_compiler_arrays_add_error)
  
} //end MFM


