#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3221_test_compiler_element2_elementwithquarkEP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Poo { System s();  Bool(3) sp(false);  Unary(4) m_i(0);  Unary(4) m_j(0);  Bits(4) m_bits(0);  Int(4) m_k(0);  Bar m_bar( Bool(1) val_b[3](false,false,false); );  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nuse System;\nelement Poo {\nSystem s;\nBool(3) sp;\nUnary(4) m_i;\nUnary(4) m_j;\nBits(4) m_bits;\nInt(4) m_k;\nBar m_bar;\nBar check(Int v) {\nBar b;\nb.val_b[v] = true;\nreturn b;}\n\n}\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool val_b[3];\nVoid reset(Bool b) {\nb = 0; }\nInt toInt(){\nreturn 3;}\n}\n");
      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn2 && rtn3 && rtn4)
	return std::string("Poo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3221_test_compiler_element2_elementwithquarkEP)
  
} //end MFM


