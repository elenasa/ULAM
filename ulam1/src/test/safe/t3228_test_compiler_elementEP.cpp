#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3228_test_compiler_elementEP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Poo { Bool(3) b(false);  Int(4) m_i(0);  Bar sbar( Bool(1) val_b[3](false,false,false); );  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = 0;\n }\n Atom aRef(Int index) native;\n Void aSet(Int index, Atom v) native;\n }\n");
      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nelement Poo {\nBool(3) b;\n Int(4) m_i;\nBar sbar;\n}\n");
      
      if(rtn2 && rtn3 && rtn4)
	return std::string("Poo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3228_test_compiler_elementEP)
  
} //end MFM


