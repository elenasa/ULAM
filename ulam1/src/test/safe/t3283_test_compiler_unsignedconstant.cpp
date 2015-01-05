#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3283_test_compiler_unsignedconstant)
  {
    std::string GetAnswerKey()
    {
      // Int i = 2147483648; prints as -2147483648, but gets this warning from gcc:
      //include/Ue_102322Fu.tcc:38:39: warning: overflow in implicit constant conversion [-Woverflow]
      // instead do the casting as in itypes.h
      /* gen output:
	 Unsigned Arg: 4294967295
	 Int Arg: 2147483647
      */
      //Ue_Fu { System s();  Int(32) test() {  Unsigned(32) u;  u 0xffffffffu cast = s ( u )print . 0 cast return } }\nExit status: 0
      return std::string("Ue_Fu { System s();  Int(32) i(2147483647);  Int(32) test() {  Unsigned(32) u;  u 0xffffffffu cast = s ( u )print . i 2147483648u cast = s ( i )print . 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt i;\nInt test(){\nUnsigned u = 0xffffffffu;\ns.print(u);\ni = (Int) 2147483648u;\ns.print(i);\nreturn 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3283_test_compiler_unsignedconstant)

} //end MFM


