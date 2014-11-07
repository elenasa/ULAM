#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3172_test_compiler_elementandquark_returnquark)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { typedef Bar Pop[2];  Int(4) m_i(0);  Bar m_bar[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,true,false); );  Int(32) test() {  Foo f;  f m_i . 15 cast = m_bar 1 [] f ( 1 cast )check . = f m_i . cast return } }\nExit status: 15
      return std::string("Ue_Foo { typedef Bar Pop[2];  Int(4) m_i(0);  Bar m_bar[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,true,false); );  Int(32) test() {  Foo f;  f m_i . 15 cast = m_bar 1 [] f ( 1 cast )check . = f m_i . cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // array of quarks, as typedef
      //modified gencode to allow quarks as return values, and parameters.
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n typedef Bar Pop[2];\n Int(4) m_i;\n Pop m_bar;\n Bar check(Int v) {\n Bar b;\n b.val_b[1] = true;\n return b;\n }\n Int test() {\n Foo f;\n f.m_i = 15;\n m_bar[1] = f.check(1);\n return f.m_i;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect
      
      // singleton quark
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Int(4) m_i; Bar m_bar; Bar check(Int v) { Bar b; b.val_b[1] = true; return b; } Int test() { Foo f; f.m_i = 15; m_bar = f.check(1); return f.m_i; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n\n");
      
      if(rtn1 && rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3172_test_compiler_elementandquark_returnquark)
  
} //end MFM


