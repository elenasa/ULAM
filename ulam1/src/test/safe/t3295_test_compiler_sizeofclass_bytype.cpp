#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3295_test_compiler_sizeofclass_bytype)
  {
    std::string GetAnswerKey()
    {
      /* gen output:
	 Int Arg: 0
	 Int Arg: 64
      */

      //node terminal proxy
      return std::string("Ue_Fu { System s();  Int(32) u(0);  Int(32) v(64);  Int(32) test() {  u 0u cast = s ( u )print . v 64u cast = s ( v )print . v return } }\nExit status: 64");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt u, v;\nInt test(){\nu = System.sizeof;\ns.print(u);\n  v = Fu.sizeof;\ns.print(v);\n return v;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3295_test_compiler_sizeofclass_bytype)

} //end MFM


