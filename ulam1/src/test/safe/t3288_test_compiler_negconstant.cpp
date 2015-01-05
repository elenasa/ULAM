#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3288_test_compiler_negconstant)
  {
    std::string GetAnswerKey()
    {
      /* gen output:
	 Int(3) Arg: 0x4  (not sign extended!)
	 Int Arg: -4
	 Int Arg: 3
	 Unsigned Arg: 4
      */
      return std::string("Ue_Fu { System s();  Int(3) n(4);  Unsigned(3) u(4);  Int(32) test() {  n -4 cast = s ( n )print . s ( -4 cast )print . s ( 3 cast )print . u n cast - cast = s ( u cast )print . 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt(3) n;\nUnsigned(3) u;\n Int test(){\n n = -4;\n s.print(n);\n s.print((Int) n.min);\n s.print((Int) n.max);\n u = -n; s.print((Unsigned) u);\nreturn 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3288_test_compiler_negconstant)

} //end MFM


