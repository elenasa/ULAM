#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3305_test_compiler_multiplestmts_binaryops)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 2
	 Int Arg: 0
	 Int Arg: 0
	 Int Arg: 1
	 Int Arg: 1
      */
      return std::string("Exit status: 1\nUe_A { System s();  Bool(7) spb(false);  Int(32) c(0);  Int(32) d(1);  Int(32) test() {  Int(32) a;  Int(32) b;  a b c 1 = = = c a b +b = s ( c )print . c c a -b b -b = s ( c )print . a c b * = s ( a )print . b b b / = s ( b )print . d a b +b c +b = s ( d )print . a b +b c +b return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // multiple statements, each of the binary ops, where all idents == 1
      // when using the same variable for each result, the final value is returned throughout.
      // to see intermediate results use different variables:
      //bool rtn1 = fms->add("A.ulam","element A{\nInt c, d;\n Int test() {\n Int a, b;\n a = b = c = 1;\n c = a + b;\n c = c - a - b;\n a = c * b;\n b = b / b;\n d = (a + b + c);\n return (a + b + c);\n }\n }\n");

      bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nBool(7) spb;\nInt c, d;\n Int test() {\n Int a, b;\n a = b = c = 1;\n c = a + b;\ns.print(c);\n c = c - a - b;\n s.print(c);\na = c * b;\ns.print(a);\n b = b / b;\n s.print(b);\nd = (a + b + c);\ns.print(d);\n return (a + b + c);\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3305_test_compiler_multiplestmts_binaryops)

} //end MFM
