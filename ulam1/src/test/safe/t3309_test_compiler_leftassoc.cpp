#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3309_test_compiler_leftassoc)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 2
       */
      // constant folded: a 2 2 / 2 * = s
      return std::string("Exit status: 2\nUe_A { Bool(7) b(false);  System s();  Int(32) a(2);  Int(32) test() {  a 2 = s ( a )print . a return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A {\n Int a;\n Int test() {\n a = 2 / 2 * 2;\n return a;\n }\n }\n"); // we want 2, not 0

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt a;\n Int test() {\n a = 2 / 2 * 2;\ns.print(a);\n return a;\n }\n }\n"); // we want 2, not 0

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3309_test_compiler_leftassoc)

} //end MFM
