#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3143_test_compiler_elementandquark)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { typedef Bar Pop[2];  Int(32) m_i(0);  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f m_i . 3 cast = f ( 1 cast )check . 7 cast return } }\nExit status: 7

      //Int(4) Arg: 0x3
      //Int(4) Arg: 0x0
      //Bool(3) Arg: 0x7 (true)

      return std::string("Ue_Foo { Bool(3) b(true);  typedef Bar Pop[2];  System s();  Int(4) m_i(0);  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f m_i . 3 cast = b f ( 1 cast )check . cast = s ( f m_i . )print . s ( m_i )print . s ( b )print . m_i cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Bar m_bar1; Int m_i; Bar m_bar2; Bar check(Bar b) { return b; } Int test() { Foo f; return 1; } }\n"); //errors quarks as parameter and return values.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { typedef Bar Pop[2]; Bar m_bar1; Int m_i; Pop m_bar2[2]; Bar m_bar3; Bool check(Int v) { return true; } Int test() { Foo f; f.check(1); return 7; } }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use Bar;\n element Foo {\n typedef Bar Pop[2];\nSystem s;\nBool(3) b;\n Int(4) m_i;\n Pop m_bar2;\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n f.m_i = 3;\n b = f.check(1);\ns.print(f.m_i);\ns.print(m_i);\ns.print(b);\n return m_i;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3143_test_compiler_elementandquark)

} //end MFM


