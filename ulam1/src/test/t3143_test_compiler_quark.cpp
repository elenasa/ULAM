#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3143_test_compiler_quark)
  {
    std::string GetAnswerKey()
    {
      return std::string(" element Foo { Bar m_bar(Bar.0.0);  Int test() {  Foo f;  1 return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Bar m_bar;  Int test() { Foo f;  return 1; } }\n");
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool valb[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3143_test_compiler_quark)
  
} //end MFM


