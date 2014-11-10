#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3183_test_compiler_element_unarycastconstant)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { Unary(4) m_u[5](1,0,3,2,0);  Int(4) m_i(2);  Bool(1) m_b(true);  Int(32) test() {  m_u 0 [] true cast = m_u 1 [] false cast = m_u 2 [] 3 cast = m_u 3 [] 2 cast = m_u 4 [] 1 - cast = m_i m_u 3 [] cast = m_b m_u 3 [] cast = m_u 3 [] cast return } }\nExit status: 2
      return std::string("Ue_Foo { System s();  Bool(3) m_b(true);  Int(4) m_i(2);  Unary(4) m_u[5](1,0,3,2,0);  Int(32) test() {  m_u 0 [] true cast = m_u 1 [] false cast = m_u 2 [] 3 cast = m_u 3 [] 2 cast = m_u 4 [] 1 cast - cast = m_i m_u 3 [] cast = s ( m_i )print . m_b m_u 3 [] cast = s ( m_b )print . m_u 3 [] cast return } }\nExit status: 2");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Unary(4) m_u[5];  Int(4) m_i;  Bool m_b;  Int test() { m_u[0] = true; m_u[1] = false; m_u[2] = 3; m_u[3] = 2; m_u[4] = -1;  m_i = m_u[3]; m_b = m_u[3];  return m_u[3]; } }\n");
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(3) m_b;\nInt(4) m_i;\n Unary(4) m_u[5];\n Int test() {\n m_u[0] = true;\n m_u[1] = false;\n m_u[2] = 3;\n m_u[3] = 2;\n m_u[4] = -1;\n  m_i = m_u[3];\ns.print(m_i);\n m_b = m_u[3];\ns.print(m_b);\n return m_u[3];\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3183_test_compiler_element_unarycastconstant)
  
} //end MFM


