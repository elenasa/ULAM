#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3241_test_compiler_unarymod)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unary(3) Arg: 0x2
	 Unary(3) Arg: 0x1
	 Unary(3) Arg: 0x0
      */
      return std::string("Exit status: 2\nUe_A { Unary(3) b(3);  System s();  Unary(3) a(2);  Unary(3) c(2);  Unary(3) d(1);  Unary(3) f(0);  Unary(4) e(4);  Int(32) test() {  a 2u cast = b 3u cast = c a cast b cast % cast = s ( c )print . d b cast a cast / cast = s ( d )print . e 4 cast = f e cast a cast % cast = s ( f )print . c cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //2 / 1 == 2 bit
      //bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; use test;  a = 2; b = 4; c = a / b; return c; } }");

      // need explicit cast (Unary(3)) using e in rhs equation:
      //./A.ulam:13:3: ERROR: Attempting to implicitly cast from RHS type: Unsigned(4), to Unary type: Unary(3) for binary operator= without casting.
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nUnary(3) a, b, c, d, f;\nUnary(4) e;\n use test;\n  a = 2u;\n b = 3u;\n c = (Unary(3)) (a % b);\ns.print(c);\nd = b / a;\ns.print(d);\ne = 4;\nf = (Unary(3)) (e % a);\ns.print(f);\n return c;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3241_test_compiler_unarymod)

} //end MFM
