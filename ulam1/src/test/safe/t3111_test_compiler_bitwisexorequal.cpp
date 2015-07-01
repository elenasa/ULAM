#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3111_test_compiler_bitwisexorequal)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 2
      */
      return std::string("Exit status: 2\nUe_A { Unary(3) b(1);  System s();  Bool(1) l(false);  Bits(3) a(2);  Int(32) test() {  a 3 cast = b 1 cast = a b cast ^= s ( a cast )print . a cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool l;\nBits(3) a;\n Unary(3) b;\nInt test() {\na = 3;\nb = 1;\na^=b;\ns.print((Int) a);\nreturn (Int) a;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3111_test_compiler_bitwisexorequal)

} //end MFM
