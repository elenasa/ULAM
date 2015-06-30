#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3184_test_compiler_element_argfunccall)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x0 (false)
	 Int(4) Arg: 0x3
       */
      return std::string("Exit status: 3\nUe_Foo { System s();  Bool(3) m_b(false);  Int(4) m_i(3);  Int(32) test() {  m_b true cast = m_b ( ( 7 7 )max )check cast = s ( m_b )print . s ( m_i )print . m_i cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i; Bool m_b; Bool check(Bool b) { if(b) m_i = 4; else m_i = 3; return b; } Bool max(Int a, Int b){ return (Bool) (a - b); } Int test() { m_b = true; m_b = check( max(7,7) ); return m_i; } }\n");
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(3) m_b;\n Int(4) m_i;\n  Bool check(Bool b) {\n if(b) m_i = 4;\n else m_i = 3;\n return b;\n }\n Bool max(Int a, Int b){\n return (a - b) != 0;\n }\n Int test() {\n m_b = true;\n m_b = check( max(7,7) );\ns.print(m_b);\ns.print(m_i);\n return m_i;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3184_test_compiler_element_argfunccall)

} //end MFM
