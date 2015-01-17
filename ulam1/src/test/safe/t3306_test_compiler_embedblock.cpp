#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3306_test_compiler_embedblock)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 5\nUe_A { Bool(1) b(false);  System s();  Unary(6) sp(0);  Int(32) j(5);  Int(32) i(0);  Int(32) test() {  j 1 3 +b cast = s ( j )print . { j j 1 cast +b = s ( j )print . Int(32) j;  j 2 4 * cast = s ( j )print . b j cast ! = s ( b ! )assert . } i b cast = s ( i )print . j return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //requires new grammar

      // includes statements after embedded block; use of variable
      // inside block defined outside then redefined inside; error
      // using variable defined inside block outside with 4 binary
      // operators; unary op; float;

      //bool rtn1 = fms->add("a.ulam","ulam\nInt j;\nInt test () {\n j = 1 + 3;\n{\nj = j + 1;\nFloat j;\nj = 2.1 * 4;\nInt j;\nBool b;\nb = !j;\n}\ni = b;\ni = 3 + b;\ni = 3 - b;\ni = 3 * b;i = 3 / b;\n} Int i;\n}\n");
      //bool rtn1 = fms->add("A.ulam","element A{\nInt j;\nBool b;\nInt test () {\n j = 1 + 3;\n{\nj = j + 1;\nInt j;\nj = 2 * 4;\nb = !j;\n}\ni = b;\n return j;\n}\n Int i;\n}\n");
      bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nUnary(6) sp;\nBool b;\nInt j;\nInt test () {\n j = 1 + 3;\ns.print(j);\n{\nj = j + 1;\ns.print(j);\nInt j;\nj = 2 * 4;\ns.print(j);\nb = !j;s.assert(!b);\n\n}\ni = b;\ns.print(i);\n return j;\n}\n Int i;\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3306_test_compiler_embedblock)

} //end MFM


