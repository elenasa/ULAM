#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3143_test_compiler_elementandquark)
  {
    std::string GetAnswerKey()
    {
      return std::string(" element Foo { Bar m_bar(Bar.0.0);  Int test() {  Foo f;  1 return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Bar m_bar;  Bar check(Bar b) { return b; } Int test() { Foo f; return 1; } }\n");
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Int val;  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3143_test_compiler_elementandquark)
  
} //end MFM


