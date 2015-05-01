#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3388_test_compiler_elementmemberselect_plusplus)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(4) Arg: 0x7
	 Int(4) Arg: 0x7
	 Int(4) Arg: 0x6
       */
      return std::string("Exit status: 6\nUe_Foo { System s();  Int(4) m_i(0);  Int(32) test() {  Foo f;  f m_i . 7 cast = s ( f m_i . )print . s ( f m_i . 1 cast += )print . s ( f m_i . 1 cast -= )print . f m_i . cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3146; tests saturation and prefix plus plus on member selected
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\n Int(4) m_i;\n Int test() {\n Foo f;\nf.m_i = 7; s.print(f.m_i);\n s.print(++f.m_i);\n s.print(--f.m_i);\n return f.m_i;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3388_test_compiler_elementmemberselect_plusplus)

} //end MFM
