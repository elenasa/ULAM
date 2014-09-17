#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3148_test_compiler_elementmemberselect_returnarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { typedef Bool(1) BA[6];  Bool(1) m_ba[6](false,true,false,true,false,true);  Int(32) test() {  Foo f;  f m_ba 0 [] . true = f m_ba . f ( 1 )check . = m_ba f m_ba . = 0 return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { typedef Bool BA[6]; BA m_ba; BA check(Int v) { BA rba; rba[1] = rba[3] = rba[5] = true; return rba; } Int test() { Foo f; f.m_ba[0] = true; f.m_ba = f.check(1); m_ba = f.m_ba; return 0; } }\n"); //2 basic member select tests: data member, func call
      
      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3148_test_compiler_elementmemberselect_returnarray)
  
} //end MFM


