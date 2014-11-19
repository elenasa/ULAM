#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3179_test_compiler_elementandquark_argcasted)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(5) m_i(0);  Bar m_bar(0);  Bool(1) m_b(true);  Int(32) test() {  Foo f;  f m_bar ( )reset . . Bool(3) t;  t true cast = m_b ( t )check = Bool(5) r;  r true cast = m_i ( r )check cast = m_b cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Bool m_b; Bool check(Bool(2+1) b) { return b /* true */; } Bool check(Bool(2+3) b) { return false; } Int test() { m_b = check( (Bool(3)) true); return m_b; } }\n"); //should be able to cast an arg..BUT NO!

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset() { val_b[1] = true; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3179_test_compiler_elementandquark_argcasted)
  
} //end MFM


