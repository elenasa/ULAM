#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3285_test_compiler_funccallargsnomatch_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Tu.ulam:7:4: ERROR: (1) <print> has no defined function with 2 matching argument types: Bool(1), Int(32), and cannot be called, while compiling class: Tu.
	Tu.ulam:8:3: ERROR: (1) <print> has no defined function with 0 matching argument types: and cannot be called.
      */
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //no matching function: error should list the type, not just the number
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nuse System;\n element Tu {\nSystem s;\nInt test(){\nBool me;\n s.print(me, 3);\ns.print();\n return 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3285_test_compiler_funccallargsnomatch_error)

} //end MFM
