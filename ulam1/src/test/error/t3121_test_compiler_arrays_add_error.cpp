#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3121_test_compiler_arrays_add_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" D.ulam:1:35: ERROR: Incompatible (nonscalar) types, LHS: <Int.16.3>, RHS: <Int.16.0> for binary operator+.\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int(16) a[3], b; Int(16) test() {return (a + b);} }");
            
      if(rtn1)
	return std::string("D.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3121_test_compiler_arrays_add_error)
  
} //end MFM


