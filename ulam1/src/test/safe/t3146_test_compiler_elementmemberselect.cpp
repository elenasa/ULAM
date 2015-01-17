#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3146_test_compiler_elementmemberselect)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_Foo { System s();  Bool(7) sp(false);  Int(32) m_i(4);  Int(32) test() {  Foo f;  f m_i . 3 cast = s ( f m_i . )print . f ( 1 cast )check . m_i 4 cast = s ( m_i )print . f m_i . return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBool(7) sp;\n Int m_i;\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n f.m_i = 3;\n f.check(1);\n m_i = 4;\n return f.m_i;\n }\n }\n"); //2 basic member select tests: data member, func call

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(7) sp;\n Int m_i;\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n f.m_i = 3;\ns.print(f.m_i);\n f.check(1);\n m_i = 4;\ns.print(m_i);\n return f.m_i;\n }\n }\n"); //2 basic member select tests: data member, func call

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3146_test_compiler_elementmemberselect)

} //end MFM


