#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3172_test_compiler_elementandquark_returnquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { typedef Bar Pop[2];  Int(4) m_i(0);  Bar m_bar[2](0,2);  Int(32) test() {  Foo f;  f m_i . 15 cast = m_bar 1 [] f ( 1 )check . = f m_i . cast return } }\nExit status: 15");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // array of quarks, as typedef
      //modified gencode to allow quarks as return values, and parameters.
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { typedef Bar Pop[2]; Int(4) m_i; Pop m_bar; Bar check(Int v) { Bar b; b.val_b[1] = true; return b; } Int test() { Foo f; f.m_i = 15; m_bar[1] = f.check(1); return f.m_i; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect
      
      // singleton quark
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Bar check(Int v) { Bar b; b.val_b[1] = true; return b; } Int test() { Foo f; f.m_i = 15; m_bar = f.check(1); return f.m_i; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3172_test_compiler_elementandquark_returnquark)
  
} //end MFM


