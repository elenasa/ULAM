#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3208_test_compiler_quark2_accessaquarkinsideaquark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Gah { Bool(1) b(false);  System m_s();  Int(3) a(0);  Int(3) d(0);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      bool rtn4 = fms->add("Gah.ulam","ulam 1;\nuse System;\nquark Gah{\nSystem m_s;\nBool b;\nInt(3) a, d;\nInt toInt(){\nif(a | 1)\nreturn (a + 4) * d;\nreturn a;\n}\nVoid set(Int xarg, Int yarg){\na=xarg;\nd=yarg;\nm_s.print(a);\nm_s.print((Int) d);\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn3 && rtn4)
	return std::string("Gah.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3208_test_compiler_quark2_accessaquarkinsideaquark)
  
} //end MFM


