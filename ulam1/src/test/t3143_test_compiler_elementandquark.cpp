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
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Bar m_bar1; Int m_i; Bar m_bar2; Bar check(Bar b) { return b; } Int test() { Foo f; return 1; } }\n"); //errors quarks as parameter and return values.
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { typedef Bar Pop[2]; Bar m_bar1; Int m_i; Pop m_bar2[2]; Bar m_bar3; Bool check(Int v) { return true; } Int test() { Foo f; return 7; } }\n");
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3143_test_compiler_elementandquark)
  
} //end MFM


