#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3304_test_compiler_unaryminus)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: -18
      */
      return std::string("Exit status: -18\nUe_A { Bool(7) b(false);  System s();  Int(32) bar(-18);  Int(32) test() {  bar -18 = Int(32) ar;  ar bar - - = s ( bar )print . bar + return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // use unary plus on return, and added 'ar' to avoid unused variable warning from g++ on generated code.
      // newline lead to valgrind detecting leak in Lexer destructor: NULLifying sourcestream ref.
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt bar;\n Int test() {\nbar = -18;\nInt ar = - - bar;\ns.print(bar);\n return +bar;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3304_test_compiler_unaryminus)

} //end MFM
