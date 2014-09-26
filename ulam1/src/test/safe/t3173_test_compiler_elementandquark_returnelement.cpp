#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3173_test_compiler_elementandquark_returnelement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(0);  Bar m_bar( Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f f ( 7 cast )check . = f m_i . cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // return element
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Foo check(Int i) { Foo f; f.m_i = i; return f; } Int test() { Foo f; f = f.check(7); return f.m_i; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3173_test_compiler_elementandquark_returnelement)
  
} //end MFM


