#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3246_test_compiler_shift)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 2
	 Unsigned Arg: 4
	 Unsigned Arg: 8
	 Unsigned Arg: 16
      */
      return std::string("Exit status: 16\nUe_A { Bool(7) b(false);  System s();  Unsigned(32) d(16);  Int(32) test() {  Int(32) a;  a 8 cast = d 1 cast = a 0 cast != cond { d d cast 1 cast << cast = s ( d )print . a a cast 1 cast >> cast = } _1: while d cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nUnsigned d;\nInt test(){Int a;\n a = 8;\nd = 1;\nwhile(a!=0){\nd = (Unsigned) (d << 1);\ns.print(d);\na = (Int) (a >> 1);\n}\nreturn (Int) d;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3246_test_compiler_shift)

} //end MFM
