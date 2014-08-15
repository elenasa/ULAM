#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3132_test_compiler_typedef_capitalization)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int t(15);  Int test() {  t ( 4 5 )times = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { typedef Int too; too times(Int m, Int n) { Int e; while( m-=1 ) e += n; e; } Int test(){\n{\nt = times(4,5); return t; } } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3132_test_compiler_typedef_capitalization)
  
} //end MFM


