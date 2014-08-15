#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3129_test_compiler_funcdefshadow)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int t(15);  Int test() {  t ( 4 5 )times = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int t; Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; return e; } Int test(){\n{Int times;\nt = times(4,5); return t; } } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3129_test_compiler_funcdefshadow)
  
} //end MFM


