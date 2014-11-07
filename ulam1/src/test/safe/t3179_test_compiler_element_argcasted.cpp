#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3179_test_compiler_element_argcasted)
  {
    std::string GetAnswerKey()
    {
      //Ue_Foo { Bool(1) m_b(true);  Int(32) test() {  m_b ( true cast )check = m_b cast return } }\nExit status: 1
      return std::string("Ue_Foo { System s();  Bool(3) m_b(true);  Int(32) test() {  m_b ( true cast )check = s ( m_b )print . m_b cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo {\n Bool m_b; Bool check(Bool(2+1) b) {\n return b /* true */;\n }\n Bool check(Bool(2+3) b) {\n return false;\n }\n Int test() {\n m_b = check( (Bool(3)) true);\n return m_b;\n }\n }\n"); //should be able to cast an arg.
 
     //changed m_b and check return type to Bool(3) to use System print for gen code test
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\n Bool(3) m_b;\n Bool(3) check(Bool(2+1) b) {\n return b /* true */;\n }\n Bool(3) check(Bool(2+3) b) {\n return false;\n }\n Int test() {\n m_b = check( (Bool(3)) true);\ns.print(m_b);\n return m_b;\n }\n }\n"); //should be able to cast an arg.

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3179_test_compiler_element_argcasted)
  
} //end MFM


