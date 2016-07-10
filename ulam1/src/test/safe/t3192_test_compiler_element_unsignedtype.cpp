#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3192_test_compiler_element_unsignedtype)
  {
    std::string GetAnswerKey()
    {

      return std::string("Exit status: 1\nUe_Foo { System s();  Unsigned(1) m_array[7](0,1,0,0,0,0,0);  Int(32) test() {  m_array 1 [] 1 cast = s ( m_array 0 [] cast )print . s ( m_array 1 [] cast )print . m_array 1 [] cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nUnsigned(1) m_array[7];\n Int test() {\n m_array[1] = 1;\ns.print((Unsigned) m_array[0]);\ns.print((Unsigned) m_array[1]);\n return m_array[1];\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3192_test_compiler_element_unsignedtype)

} //end MFM


