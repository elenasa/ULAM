#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3490_test_compiler_unaryadd2unsigned_error)
  {
    std::string GetAnswerKey()
    {
      /* gencode output:
	 Unary(3) Arg: 0x3
      */
      return std::string("Exit status: 3\nUe_A { constant Unsigned(3) b = 5;  System s();  constant Unsigned(3) a = 7;  constant Unary(3) c = 3;  Int(32) test() {  s ( 7u )print . 7u cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nUnary(3) c;\n use test;\n Unsigned(3) a = 7u, b = 5u;\n c = (Unary(3)) (a + b);\ns.print(c);\n return c;\n }\n }\n");

      // try as named constants
      // without explicit cast of (a+b) we get error:
      //./A.ulam:5:11: ERROR: Constant value expression for (c = 7u) is not representable as Unary(3).
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nconstant Unsigned(3) a = 7u; constant Unsigned(3) b = 5u;\n constant Unary(3) c = (a + b);\n use test;\n s.print(c);\n return c;\n }\n }\n");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3490_test_compiler_unaryadd2unsigned_error)

} //end MFM
