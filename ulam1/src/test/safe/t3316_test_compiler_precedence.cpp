#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3316_test_compiler_precedence)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 7\nUe_A { Bool(7) b(false);  System s();  Int(32) j(7);  Int(32) test() {  j 1 2 3 * +b cast = s ( j )print . j return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt j;\n Int test() {\n j = 1 + 2 * 3;\ns.print(j);\n return j;\n }\n }\n"); // precedence test

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3316_test_compiler_precedence)

} //end MFM


