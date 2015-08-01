#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3404_test_compiler_negnamedconstant)
  {
    std::string GetAnswerKey()
    {
      /* gen output:
	 Int(3) Arg: 0x5
	 Int Arg: -3
	 Int(3) Arg: 0x4
	 Int(3) Arg: 0x3
	 Unsigned Arg: 3
      */
      return std::string("Exit status: 0\nUe_Fu { System s();  constant Int(3) n = -3;  Unsigned(3) u(3);  Int(32) test() {  s ( -3 )print . s ( -3 cast )print . s ( -4 )print . s ( 3 )print . u 3 cast = s ( u cast )print . 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3288
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nconstant Int(3) n = -3;\nUnsigned(3) u;\n Int test(){\n s.print(n);\n s.print((Int) n);\n s.print( n.minof);\n s.print( n.maxof);\n u = -n; s.print((Unsigned) u);\n return 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3404_test_compiler_negnamedconstant)

} //end MFM
