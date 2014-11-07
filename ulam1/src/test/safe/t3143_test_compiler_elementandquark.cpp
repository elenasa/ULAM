#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3143_test_compiler_elementandquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { typedef Bar Pop[2];  Int(32) m_i(0);  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f m_i . 3 cast = f ( 1 cast )check . 7 cast return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Bar m_bar1; Int m_i; Bar m_bar2; Bar check(Bar b) { return b; } Int test() { Foo f; return 1; } }\n"); //errors quarks as parameter and return values.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { typedef Bar Pop[2]; Bar m_bar1; Int m_i; Pop m_bar2[2]; Bar m_bar3; Bool check(Int v) { return true; } Int test() { Foo f; f.check(1); return 7; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n typedef Bar Pop[2];\n Int m_i;\n Pop m_bar2[2];\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n f.m_i = 3;\n f.check(1);\n return 7;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");
      
      if(rtn1 & rtn2)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3143_test_compiler_elementandquark)
  
} //end MFM


