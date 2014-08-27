#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3142_test_compiler_element)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int a(0);  Int test() {  Foo f;  1 return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int a;  Int test() { Foo f;  return 1; } }\n");
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3142_test_compiler_element)
  
} //end MFM


