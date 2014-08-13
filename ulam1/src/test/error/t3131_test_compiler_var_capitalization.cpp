#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3131_test_compiler_var_capitalization)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int t(15);  Int main() {  t ( 4 5 )times = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int T; Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; e; } Int main(){\n{\nt = times(4,5); } } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3131_test_compiler_var_capitalization)
  
} //end MFM


