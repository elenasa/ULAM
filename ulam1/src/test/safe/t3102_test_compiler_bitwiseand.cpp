#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3102_test_compiler_bitwiseand)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x2
	 assert: arg is 1
	 after assert's abort: arg is 1
      */
      return std::string("Exit status: 2\nUe_A { Int(3) b(2);  System s();  Bool(1) d(true);  Int(3) a(2);  Int(32) test() {  a 3 cast = b 2 cast = a a cast b cast & cast = s ( a )print . d a cast = s ( d )assert . a cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {

      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool d;\nInt(3) a, b;\nInt test() {\na = 3;\nb = 2;\na = (Int(3)) ((Bits(3)) a & (Bits(3)) b);\ns.print(a);\n d = (Bool) a;\n s.assert(d);\nreturn a;\n}\n}\n");

      // without system for testing
      //bool rtn1 = fms->add("A.ulam","ulam 1;\n element A {\nBool d;\nInt(3) a, b;\n Int test() {\na = 3;\nb = 2;\nd = a = a & b;\nreturn a;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3102_test_compiler_bitwiseand)

} //end MFM
