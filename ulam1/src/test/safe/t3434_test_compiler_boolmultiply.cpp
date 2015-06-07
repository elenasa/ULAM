#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3434_test_compiler_boolmultiply)
  {
    std::string GetAnswerKey()
    {
      /* gencode output:
	 Unary(3) Arg: 0x1
	 Bool(3) Arg: 0x7 (true)
	 Unary(3) Arg: 0x1
      */
      return std::string("Exit status: 1\nUe_A { Bool(1) b(true);  System s();  Bool(3) a(true);  Bool(3) c(true);  Unary(3) d(1);  Unary(3) e(1);  Int(32) test() {  a true cast = d b a cast 3 cast * cast = cast = s ( d )print . e c a cast 3 cast * cast = cast = s ( c )print . s ( d )print . b cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //here's what happens when we try to multiple bools and save in a bool;
      // note1: cast as a unary the product is true (==1)
      // note2: cast as an Int(32), the exit status is 1
      // cannot multiply 1-bit Bool * 3 since 3 doesn't fit in 3 bits.
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(3) a, c;\n Bool b;\nUnary(3) d,e;\nInt test() {\na = true;\n d = b = a * 3;\n s.print(d);\n e = c = a * 3;\n s.print(c);\n s.print(d);\n return b;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {Void print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3434_test_compiler_boolmultiply)

} //end MFM
