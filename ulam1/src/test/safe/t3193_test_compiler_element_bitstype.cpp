#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3193_test_compiler_element_bitstype)
  {
    std::string GetAnswerKey()
    {

      //Ue_Foo { Bits(3) m_i[3](1,2,3);  Bool(3) m_b[3](false,false,true);  Int(32) test() {  m_i 0 [] 1 cast = m_i 1 [] 2 cast = m_i 2 [] m_i 0 [] m_i 1 [] | = m_b 0 [] m_i 0 [] cast = m_b 1 [] m_i 1 [] cast = m_b 2 [] m_i 2 [] cast = m_i 2 [] cast return } }\nExit status: 3

      return std::string("Ue_Foo { System s();  Bool(1) sp(false);  Bits(3) m_i[3](1,2,3);  Bool(3) m_b[3](false,false,true);  Int(32) test() {  m_i 0 [] 1 cast = m_i 1 [] 2 cast = m_i 2 [] m_i 0 [] m_i 1 [] | = m_b 0 [] m_i 0 [] cast = m_b 1 [] m_i 1 [] cast = m_b 2 [] m_i 2 [] cast = s ( m_i 0 [] cast )print . s ( m_i 1 [] cast )print . s ( m_i 2 [] cast )print . s ( m_b 0 [] )print . s ( m_b 1 [] )print . s ( m_b 2 [] )print . m_i 2 [] cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nelement Foo {\nSystem s;\nBool sp;\n Bits(3) m_i[3];\n Bool(3) m_b[3];\n Int test() { m_i[0] = 1;\n m_i[1] = 2;\n m_i[2] = m_i[0] | m_i[1];\n m_b[0] = m_i[0];\n m_b[1] = m_i[1];\n m_b[2] = m_i [2];\ns.print((Unsigned) m_i[0]);\ns.print((Unsigned) m_i[1]);\ns.print((Unsigned) m_i[2]);\ns.print(m_b[0]);\ns.print(m_b[1]);\ns.print(m_b[2]);\n return m_i[2];\n } }\n"); 


      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3193_test_compiler_element_bitstype)
  
} //end MFM


