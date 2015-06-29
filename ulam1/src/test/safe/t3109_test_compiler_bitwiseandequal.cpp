#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3109_test_compiler_bitwiseandequal)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 3
      */
      return std::string("Exit status: 3\nUe_A { Unary(3) b(2);  System s();  Bool(1) l(false);  Bits(3) a(3);  Int(32) test() {  a 3 cast = b 2 cast = a b cast &= s ( a cast )print . a cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //./A.ulam:7:2: ERROR: Bits is the supported type for bitwise operator&=.
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool l;\nBits(3) a;\n Unary(3) b;\n Int test() {a = 3;\nb = 2;\na&=b;\ns.print((Int) a);\nreturn (Int) a;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3109_test_compiler_bitwiseandequal)

} //end MFM
