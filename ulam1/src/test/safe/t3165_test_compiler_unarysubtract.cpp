#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3165_test_compiler_unarysubtract)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unary(3) Arg: 0x1
	 Unary(3) Arg: 0x0
      */
      return std::string("Exit status: 1\nUe_A { Unary(3) b(1);  System s();  Bool(1) sp(false);  Unary(3) a(2);  Unary(3) c(1);  Unary(3) d(0);  Int(32) test() {  a 2 cast = b 1 cast = c a cast b cast -b cast = s ( c )print . d b cast a cast -b cast = s ( d )print . c cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //2 - 1 == 1 bit; for negative unary, d is 0
      //bool rtn1 = fms->add("A.ulam","use System;\nelement A { Unary(3) a, b, c, d;\n use test;\n  a = 2;\n b = 1;\n c = a - b;\n d = b - a;\n return c;\n } }");

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool sp;\nUnary(3) a, b, c, d;\n use test;\n  a = 2;\n b = 1;\n c = a - b;\ns.print(c);\n d = b - a;\ns.print(d);\n return c;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3165_test_compiler_unarysubtract)

} //end MFM
