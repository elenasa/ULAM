#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3122_test_compiler_opequal_invalidlhs_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a[3](1,0,0);  Int test() {  a 0 [] 1 = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a[3]; Int test() { 3 = a[1]; a[0] = 1; return a[1]; } }");
            
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3122_test_compiler_opequal_invalidlhs_error)
  
} //end MFM


