#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3323_test_compiler_namedconstant_scope)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 4
	 Int Arg: 7
	 Int Arg: 8
	 Int Arg: 7
       */
      return std::string("Exit status: 7\nUe_A { System s();  Int(32) j(7);  Int(32) i(7);  Int(32) test() {  3 = cOW const j 1 3 +b cast = s ( j )print . { j j 3 cast +b = s ( j )print . Int(32) j;  4 = cOW const j 2 4 * cast = s ( j )print . } i j = s ( i )print . j return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //requires new grammar: tests scope

      // includes statements after embedded block; use of variable
      // inside block defined outside then redefined inside;

      //basic:
      //      bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nInt j;\nInt test () {\nconstant Int cOW = 3;\n j = 1 + cOW;\ns.print(j);\n return j;\n}\n}\n");

      bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nInt j;\nInt test () {\nconstant Int cOW = 3;\n j = 1 + cOW;\ns.print(j);\n{\nj = j + cOW;\ns.print(j);\nInt j;\nconstant Int cOW = 4;\nj = 2 * cOW;\ns.print(j);\n}\ni = j;\ns.print(i);\n return j;\n}\n Int i;\n}\n");


      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3323_test_compiler_namedconstant_scope)

} //end MFM


