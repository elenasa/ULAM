#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3185_test_compiler_elementandquark_coordindexfunc)
  {
    std::string GetAnswerKey()
    {

      /* gen code output:
	 Int Arg: 40
       */
      return std::string("Exit status: 40\nUe_Foo { System s();  Bool(7) m_b(false);  Int(32) m_idx(40);  C2D m_coord( Int(6) m_width(9);  Int(6) m_height(4); );  Int(32) test() {  m_coord ( )init . m_idx m_coord ( 0 0 )getIndex . = s ( m_idx )print . m_idx return } }\nUq_System { <NOMAIN> }\nUq_C2D { Int(6) m_width(0);  Int(6) m_height(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use C2D; element Foo { C2D m_coord; Int m_idx; Bool m_b; Int test() { m_coord.init(); m_idx = m_coord.getIndex(0,0);  return m_idx; } }\n");

      //bool rtn2 = fms->add("C2D.ulam","quark C2D { Int(4) m_width; Int(4) m_height;  Void init(Int x, Int y) { m_width = x; m_height = y; return; } Void init() { m_width = 9; m_height = 4; return; /* event window overload */ } Int getIndex(Int a, Int b){return ((m_height-b) * m_width + (m_height-a)); } }\n");

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n use C2D;\n element Foo {\nSystem s;\nBool(7) m_b;\n Int m_idx;\n C2D m_coord;\n Int test() {\n m_coord.init();\n m_idx = m_coord.getIndex(0,0);\ns.print(m_idx);\n return m_idx;\n }\n }\n");

      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n Int(6) m_width;\n Int(6) m_height;\n  Void init(Int x, Int y) {\n m_width = x;\n m_height = y;\n return;\n }\n Void init() {\n m_width = 9;\n m_height = 4;\n return;\n /* event window overload */ }\n Int getIndex(Int a, Int b){\nreturn ((m_height-b) * m_width + (m_height-a));\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3185_test_compiler_elementandquark_coordindexfunc)

} //end MFM
