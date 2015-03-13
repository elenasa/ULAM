#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3371_test_compiler_elementandquarkwargs_hascond)
  {
    std::string GetAnswerKey()
    {
      /* gen code output: (correct answer unlike eval exit status)
	 Int Arg: 1
      */
      return std::string("Exit status: 0\nUe_Eltypo { System s();  Typo(1) t( constant Int(32) a = 1; );  Int(32) test() {  self Typo(1) has cond { s ( t 1 . )print . t 1 . return } if 0 cast return } }\nUq_Typo { /* empty class block */ }Uq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //quark misspelled
      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nquark Typo(Int a) {\n }\n");

      bool rtn2 = fms->add("Eltypo.ulam", "ulam 1;\nuse Typo;\nuse System;\n element Eltypo {\nSystem s;\n Typo(1) t;\n Int test(){\n if(self has Typo(1)){\n s.print(t.a);\n return t.a;\n}\n return 0;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Eltypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3371_test_compiler_elementandquarkwargs_hascond)

} //end MFM
