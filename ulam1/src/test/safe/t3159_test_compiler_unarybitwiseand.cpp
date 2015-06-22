#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3159_test_compiler_unarybitwiseand)
  {
    std::string GetAnswerKey()
    {
      /* gencode output:
	Unary(3) Arg: 0x1
	Unsigned Arg: 0
      */
      return std::string("Exit status: 1\nUe_A { Unary(3) b(1);  System s();  Unary(3) a(2);  Unary(3) c(1);  Unsigned(2) e(2);  Unsigned(2) f(1);  Unsigned(2) g(0);  Int(32) test() {  a e 2 cast = cast = b f 1 cast = cast = c a b & = s ( c )print . g e f & = s ( g cast )print . c cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // a is 2 bits; b is 1 bit; a & b = 1 bit as Unary.3 (c); notice as Int.3 (g), 2 & 1 = 0
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nUnary(3) a, b, c;\n Unsigned(2) e, f, g;\n use test;\n  a = e = 2;\n b = f = 1;\n c = a & b;\ns.print(c);\ng = e & f;\ns.print((Unsigned) g);\n return c;\n }\n }\n");
      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3159_test_compiler_unarybitwiseand)

} //end MFM
