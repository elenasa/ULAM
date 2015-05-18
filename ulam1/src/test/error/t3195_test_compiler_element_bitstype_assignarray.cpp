#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3195_test_compiler_element_bitstype_assignarray)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:12:6: ERROR: Array casts currently not supported.
	./Foo.ulam:12:6: ERROR: Cannot CAST type: Bits(3)[3] as a Bool(3)[3].
      */
      return std::string("Exit status: 3\nUe_Foo { Bits(3) m_i[3](1,2,3);  Bits(3) m_b[3](1,2,3);  Int(32) test() {  m_i 0 [] 1 cast = m_i 1 [] 2 cast = m_i 2 [] m_i 0 [] cast m_i 1 [] cast +b cast = m_b m_i = m_b 2 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // not to mention different types! error message. array cast not implemented. try same type OK.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;  element Foo { Bits(3) m_i[3]; Bits(3) m_b[3]; Int test() { m_i[0] = 1; m_i[1] = 2; m_i[2] = m_i[0] + m_i[1]; m_b = m_i; return m_b[2]; } }\n");

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nelement Foo {\nSystem s;\nBool sp;\n Bits(3) m_i[3];\n Bool(3) m_b[3];\n Int test() {\n m_i[0] = 1;\n m_i[1] = 2;\n m_i[2] = m_i[0] + m_i[1];\n m_b = m_i;\ns.print((Unsigned) m_i[0]);\ns.print((Unsigned) m_i[1]);\ns.print((Unsigned) m_i[2]);\ns.print(m_b[0]);\ns.print(m_b[1]);\ns.print(m_b[2]);\n return m_b[2];\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3195_test_compiler_element_bitstype_assignarray)

} //end MFM
