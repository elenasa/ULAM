#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3198_test_compiler_elementandquark_usercasttoquark_error)
  {
    std::string GetAnswerKey()
    {
      //Foo.ulam:1:90: ERROR: Cannot cast to type <Bar.0.-1>.
      return std::string("Ue_Foo { Int(4) m_i(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 cast )check = m_i m_bar ( )toInt . cast = m_i cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Bool(3) m_b; Bar m_bar; Int test() { m_b = true; m_bar = (Bar) m_b; return m_b; } }\n"); 
      
      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } Int toInt(){ return 7;} }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3198_test_compiler_elementandquark_usercasttoquark_error)
  
} //end MFM


