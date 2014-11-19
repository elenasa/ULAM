#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3131_test_compiler_var_capitalization)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_D { Int T(15);  Int test() {  T ( 4 5 )times = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int T; Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; return e; } Int test(){\n{\nT = times(4,5); return T; } } }");
      
      if(rtn1)
	return std::string("D.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3131_test_compiler_var_capitalization)
  
} //end MFM


