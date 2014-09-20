#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3181_test_compiler_element_argcastconstant)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(5) m_i(0); Bool(1) m_b(true);  Int(32) test() {  Foo f;  f m_bar ( )reset . . Bool(3) t;  t true cast = m_b ( t )check = Bool(5) r;  r true cast = m_i ( r )check cast = m_b cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i; Bool m_b; Bool check(Bool(2+1) b) { return b /* true */; } Int test() { m_b = check(true); return m_b; } }\n"); //should be able to cast an arg..BUT NO!

      if(rtn1)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3181_test_compiler_element_argcastconstant)
  
} //end MFM


