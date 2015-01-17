#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3180_test_compiler_element_argdatamember)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_Foo { System s();  Bits(2) sp(0);  Bool(1) m_b(false);  Int(4) m_i(0);  Int(32) test() {  Foo f;  f m_b . true cast = m_b f ( m_b )check . = s ( m_i )print . s ( f m_i . )print . f m_i . cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i; Bool m_b; Bool check(Bool b) { if(b) m_i = 4; else m_i = 3; return b; } Int test() { Foo f; f.m_b = true; m_b = f.check(m_b); return f.m_i; } }\n");

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBits(2) sp;\nBool m_b;\n Int(4) m_i;\n  Bool check(Bool b) {\n if(b) m_i = 4;\n else m_i = 3;\n return b;\n }\n Int test() {\n Foo f;\n f.m_b = true;\n m_b = f.check(m_b);\ns.print(m_i);\ns.print(f.m_i);\n return f.m_i;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3180_test_compiler_element_argdatamember)

} //end MFM


