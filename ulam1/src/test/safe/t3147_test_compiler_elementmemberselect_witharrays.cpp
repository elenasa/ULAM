#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3147_test_compiler_elementmemberselect_witharrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(1) m_ba[6](false,true,false,false,false,true);  Int(32) test() {  Foo f;  f m_ba 1 [] . true cast = f m_ba 5 [] . f ( 1 cast )check . = m_ba f m_ba . = 0 cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\n Bool m_ba[6];\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n f.m_ba[1] = true;\n f.m_ba[5] = f.check(1);\n m_ba = f.m_ba;\n return 0;\n }\n }\n"); //2 basic member select tests: data member, func call
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3147_test_compiler_elementmemberselect_witharrays)
  
} //end MFM


