#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3306_test_compiler_embedblock)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 4
	 Int Arg: 5
	 Int Arg: 8
	 assert: arg is 1
	 after assert's abort: arg is 1
	 Int Arg: 5
       */
      // constant folded: 3 1 +b, and 2 4 *,
      return std::string("Exit status: 5\nUe_A { Bool(1) b(true);  System s();  Unary(6) sp(0);  Int(32) j(5);  Int(32) i(5);  Int(32) test() {  j 4 = s ( j )print . { j j 1 +b = s ( j )print . Int(32) j;  j 8 = s ( j )print . b j 8 == = s ( b )assert . } i j = s ( i )print . j return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //requires new grammar: tests scope

      // includes statements after embedded block; use of variable
      // inside block defined outside then redefined inside;

      bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nUnary(6) sp;\nBool b;\nInt j;\nInt test () {\n j = 1 + 3;\ns.print(j);\n{\nj = j + 1;\ns.print(j);\nInt j;\nj = 2 * 4;\ns.print(j);\nb = (j == 8);s.assert(b);\n\n}\ni = j;\ns.print(i);\n return j;\n}\n Int i;\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3306_test_compiler_embedblock)

} //end MFM
