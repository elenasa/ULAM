#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3142_test_compiler_element)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { Int(32) a(0);  Int(32) test() {  Foo f;  1 cast return } }\nExit status: 1
      return std::string("Ue_Foo { Bool(7) sp(false);  Int(32) a(0);  Int(32) test() {  Foo f;  1 cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo {\nBool(7) sp;\n Int a;\n Int test() {\n Foo f;\n  return 1;\n }\n }\n");
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3142_test_compiler_element)
  
} //end MFM


