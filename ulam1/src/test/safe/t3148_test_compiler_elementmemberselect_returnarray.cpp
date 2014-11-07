#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3148_test_compiler_elementmemberselect_returnarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { typedef Bool(1) BA[6];  Bool(1) m_ba[6](false,true,false,true,false,true);  Int(32) test() {  Foo f;  f m_ba 0 [] . true cast = f m_ba . f ( 1 cast )check . = m_ba f m_ba . = 0 cast return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\n typedef Bool BA[6];\n BA m_ba;\n BA check(Int v) {\n BA rba;\n rba[1] = rba[3] = rba[5] = true;\n return rba;\n }\n Int test() {\n Foo f;\n f.m_ba[0] = true;\n f.m_ba = f.check(1);\n m_ba = f.m_ba;\n return 0;\n }\n }\n"); //2 basic member select tests: data member, func call
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3148_test_compiler_elementmemberselect_returnarray)
  
} //end MFM


