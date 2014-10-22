#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3206_test_compiler_quark_systemnativefuncs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(3) b(false);  Int(4) i(0);  Int(4) j(15);  Int(32) test() {  i 0 cast = j ( i cast 1 cast -b )update = j cast return } }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nelement Foo {\nBool(3) b;\nInt(4) i, j;\nInt(4) update(Int x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update(i - 1);\n\nreturn j;\n}\n}\n");

      // test system quark with native overloaded print funcs
      bool rtn2 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn2)
	return std::string("System.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3206_test_compiler_quark_systemnativefuncs)
  
} //end MFM


