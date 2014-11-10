#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3196_test_compiler_elementandquark_usercastquark)
  {
    std::string GetAnswerKey()
    {
      //extra casts..
      //Ue_Foo { Int(4) m_i(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 cast )check = m_i m_bar ( )toInt . cast = m_i cast return } }\nExit status: 7
      return std::string("Ue_Foo { Int(4) m_i(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 cast )check = m_i m_bar ( )toInt . cast cast = m_i cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Bar check(Int v) { Bar b; b.val_b[1] = true; return b; } Int test() { m_bar = check(1); m_i = (Int(4)) m_bar; return m_i; } }\n");
      
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } Int toInt(){ return 7;} }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3196_test_compiler_elementandquark_usercastquark)
  
} //end MFM


