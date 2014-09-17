#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3147_test_compiler_elementmemberselect_witharrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(1) m_ba[6](false,true,false,false,false,true);  Int(32) test() {  Foo f;  f m_ba 1 [] . true = f m_ba 5 [] . f ( 1 )check . = m_ba f m_ba . = 0 return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Bool m_ba[6]; Bool check(Int v) { return true; } Int test() { Foo f; f.m_ba[1] = true; f.m_ba[5] = f.check(1); m_ba = f.m_ba; return 0; } }\n"); //2 basic member select tests: data member, func call
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3147_test_compiler_elementmemberselect_witharrays)
  
} //end MFM


