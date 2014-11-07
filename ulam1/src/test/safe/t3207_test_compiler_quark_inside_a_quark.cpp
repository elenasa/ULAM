#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3207_test_compiler_quark_inside_a_quark)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(3) b(false);  Int(4) i(0);  Int(4) j(15);  Int(32) test() {  i 0 cast = j ( i cast 1 cast -b )update = j cast return } }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nuse System;\nuse Gah;\nquark Bar{\nSystem m_s;\nBool b;\nUnsigned(3) x, y;\nGah m_gah;\nInt toInt(){\nif(b)\nreturn (x * 4) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=xarg;\ny=yarg;\nm_s.print((Unsigned) x);\nm_s.print((Unsigned) y);\n\nif(yarg)\n{\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      bool rtn4 = fms->add("Gah.ulam","ulam 1;\nuse System;\nquark Gah{\nSystem m_s;\nBool b;\nInt(3) a, d;\nInt toInt(){\nif(a | 1)\nreturn (a + 4) * d;\nreturn a;\n}\nVoid set(Int xarg, Int yarg){\na=xarg;\nd=yarg;\n}\n}\n");

      if(rtn2 && rtn3 && rtn4)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3207_test_compiler_quark_inside_a_quark)
  
} //end MFM


