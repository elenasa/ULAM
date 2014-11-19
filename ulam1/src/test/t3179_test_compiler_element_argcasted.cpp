#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3179_test_compiler_element_argcasted)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(1) m_b(true);  Int(32) test() {  m_b ( true cast )check = m_b cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Bool m_b; Bool check(Bool(2+1) b) { return b /* true */; } Bool check(Bool(2+3) b) { return false; } Int test() { m_b = check( (Bool(3)) true); return m_b; } }\n"); //should be able to cast an arg.

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3179_test_compiler_element_argcasted)
  
} //end MFM


