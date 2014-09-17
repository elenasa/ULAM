#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3120_test_compiler_funcdef_emptytest_error)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_D { Int(16) a[3](0,0,0);  Int(16) b(0);  Void test() {  {} } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int(16) a[3], b; Void test() { } }");
      
      if(rtn1)
	return std::string("D.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3120_test_compiler_funcdef_emptytest_error)
  
} //end MFM


