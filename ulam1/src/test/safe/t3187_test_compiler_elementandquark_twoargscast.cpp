#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3187_test_compiler_elementandquark_twoargscast)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { Int(32) m_idx(40);  Int(4) m_x(9);  Int(4) m_y(4);  Int(32) test() {  m_x 9 cast = m_y 4 cast = m_idx ( m_x cast m_y cast )func = m_x cast m_y cast * return } }\nExit status: 36
      return std::string("Ue_Foo { System s();  Bool(7) sp(false);  Int(32) m_idx(40);  Int(6) m_x(9);  Int(6) m_y(4);  Int(32) test() {  m_x 9 cast = m_y 4 cast = m_idx ( m_x cast m_y cast )func = s ( m_idx )print . s ( m_x cast m_y cast * )print . m_x cast m_y cast * return } }\nExit status: 36");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use C2D;\nuse System;\n element Foo {\nSystem s;\nBool(7) sp;\n Int m_idx;\n Int(6) m_x, m_y;\n Int func(Int i, Int j) {\n C2D c;\n  c.init(i,j);\n return c.getIndex(0,0);\n }\n Int test() {\n m_x = 9;\n m_y = 4;\n  m_idx = func((Int) m_x,(Int) m_y);\ns.print(m_idx);\ns.print(m_x * m_y);\n return (m_x * m_y);\n }\n }\n"); //cast args; exit status (36)

      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n Int(6) m_width;\n Int(6) m_height;\n  Void init(Int x, Int y) {\n m_width = x;\n m_height = y;\n return;\n }\n Void init() {\n m_width = 9;\n m_height = 4;\n return;\n /* event window overload */ }\n Int getIndex(Int a, Int b){\n return ((m_height-b) * m_width + (m_height-a));\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3187_test_compiler_elementandquark_twoargscast)
  
} //end MFM


