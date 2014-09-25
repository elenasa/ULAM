#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3174_test_compiler_elementandquark_argelement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Int(4) m_i(0);  Bar m_bar(0);  Int(32) test() {  Int(32) i;  Foo f;  f m_i . 5 cast = i ( f )check = i return } }\nExit status: 5");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // return element
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Int check(Foo f) { return f.m_i; } Int test() { Int i; Foo f; f.m_i = 5; i = check(f); return i; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3174_test_compiler_elementandquark_argelement)
  
} //end MFM


