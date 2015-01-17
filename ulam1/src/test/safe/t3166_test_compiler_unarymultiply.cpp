#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3166_test_compiler_unarymultiply)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_A { Unary(3) b(2);  System s();  Bool(1) sp(false);  Unary(3) a(2);  Unary(3) c(3);  Int(32) test() {  a 2 cast = b 2 cast = c a cast b cast * cast = s ( c )print . c cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //2 * 2 == 4 bit -> maxes out 3 bit unary == 3
      //bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; use test;  a = 2; b = 2; c = a * b; return c; } }");

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool sp;\nUnary(3) a, b, c;\n use test;\n  a = 2;\n b = 2;\n c = a * b;\ns.print(c);\n return c;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3166_test_compiler_unarymultiply)

} //end MFM


