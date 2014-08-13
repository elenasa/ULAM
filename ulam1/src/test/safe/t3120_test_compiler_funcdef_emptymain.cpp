#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3120_test_compiler_funcdef_emptymain)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[3](0,0,0);  Int b(0);  Int main() {  {} } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[3], b; Int main() { } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3120_test_compiler_funcdef_emptymain)
  
} //end MFM


