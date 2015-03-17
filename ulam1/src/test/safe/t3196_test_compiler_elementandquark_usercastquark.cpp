#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3196_test_compiler_elementandquark_usercastquark)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(4) Arg: 0x7
      */
      return std::string("Exit status: 7\nUe_Foo { System s();  Bool(3) sp(false);  Int(4) m_i(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 )check = m_i m_bar ( )toInt . cast cast = s ( m_i )print . m_i cast return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use Bar;\n element Foo {\nSystem s;\nBool(3) sp;\n Int(4) m_i;\n Bar m_bar;\n Bar check(Int v) {\n Bar b;\n b.val_b[1] = true;\n return b;\n }\n Int test() {\n m_bar = check(1);\n m_i = (Int(4)) m_bar;\ns.print(m_i);\n return m_i;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Int toInt(){\n return 7;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3196_test_compiler_elementandquark_usercastquark)

} //end MFM
