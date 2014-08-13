#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3129_test_compiler_funcdefshadow)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int t(15);  Int main() {  t ( 4 5 )times = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int t; Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; e; } Int main(){\n{Int times;\nt = times(4,5); } } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3129_test_compiler_funcdefshadow)
  
} //end MFM


