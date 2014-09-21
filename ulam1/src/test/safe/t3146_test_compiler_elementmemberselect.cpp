#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3146_test_compiler_elementmemberselect)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(32) m_i(4);  Int(32) test() {  Foo f;  f m_i . 3 cast = f ( 1 cast )check . m_i 4 cast = f m_i . return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int m_i; Bool check(Int v) { return true; } Int test() { Foo f; f.m_i = 3; f.check(1); m_i = 4; return f.m_i; } }\n"); //2 basic member select tests: data member, func call
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3146_test_compiler_elementmemberselect)
  
} //end MFM


