#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3181_test_compiler_element_argcastconstant)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_Foo { System s();  Bool(3) m_b(true);  Int(4) m_i(1);  Int(32) test() {  m_i true cast = Bool(3) b;  b true cast = m_b ( true cast )check cast = s ( m_b )print . m_b cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i; Bool m_b; Bool check(Bool(2+1) b) { return b /* true */; } Int test() { m_i = true; Bool(3) b; b = true; m_b = check( true /*b*/); return m_b; } }\n"); //constant Args cast automatically

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\n Bool(3) m_b;\n Int(4) m_i;\n  Bool check(Bool(2+1) b) {\n return b /* true */;\n }\n Int test() {\n m_i = true;\n Bool(3) b;\n b = true;\n m_b = check( true /*b*/);\ns.print(m_b);\n return m_b;\n }\n }\n"); //constant Args cast automatically

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3181_test_compiler_element_argcastconstant)

} //end MFM


