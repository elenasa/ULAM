#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3214_test_compiler_plusequaloverflow)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x3
	 Int(3) Arg: 0x3
	 assert: arg is 1
	 after assert's abort: arg is 1
      */

      return std::string("Exit status: 0\nUe_A { Int(3) b(3);  System s();  Bool(1) d(false);  Int(3) a(3);  Int(3) c(3);  Int(32) test() {  a 3 = b 3 = c a cast b cast +b cast = s ( c )print . a b += s ( a )print . d a cast c cast -b 0 cast != = s ( d ! )assert . d cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool d;\nInt(3) a, b, c;\nInt test() {\na = 3;\nb = b.maxof; //was 4\nc =(Int(3)) (a + b);\ns.print(c);\na+=b;\ns.print(a);\nd=(a-c)!=0;\ns.assert(!d);\nreturn d;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3214_test_compiler_plusequaloverflow)

} //end MFM
