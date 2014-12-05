#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3246_test_compiler_shift)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 16
      */
      return std::string("Ue_A { Bool(7) b(false);  System s();  Int(32) d(16);  Int(32) test() {  Int(32) a;  a 8 cast = d 1 cast = a cast cond { d d 1 cast << = s ( d )print . a a 1 cast >> = } while d return } }\nExit status: 16");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt d;\nInt test(){Int a;\n a = 8;\nd = 1;\nwhile(a){\nd = d << 1;\ns.print(d);\na = a >> 1;\n}\nreturn d;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3246_test_compiler_shift)

} //end MFM


