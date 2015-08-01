#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3398_test_compiler_elementandunion_hascond)
  {
    std::string GetAnswerKey()
    {
      /* gen code output: (correct answer unlike eval exit status)
	 Int Arg: 1
      */
      return std::string("Exit status: 1\nUe_Eltypo { System s();  Typo(1) t( constant Int(32) a = 1;  Unsigned(1) u(0);  Int(2) i(0); );  Int(32) test() {  Eltypo el;  el Typo(1) has cond { s ( t 1 . )print . t 1 . return } if 2u cast return } }\nUq_Typo { constant Int(32) a = NONREADYCONST;  Unsigned(UNKNOWN) u(0);  Int(UNKNOWN) i(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //uses union instead of quark:
      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nunion Typo(Int a) {\nUnsigned(a) u;\nInt(a+1) i;\n }\n");

      bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\nuse Typo;\nuse System;\n element Eltypo {\nSystem s;\n Typo(1) t;\n Int test(){\nEltypo el;\n if(el has Typo(1)){\n s.print(t.a);\n return t.a; \n}\n return t.sizeof;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Eltypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3398_test_compiler_elementandunion_hascond)

} //end MFM
