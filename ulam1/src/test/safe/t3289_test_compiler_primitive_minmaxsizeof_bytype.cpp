#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3289_test_compiler_primitive_minmaxsizeof_bytype)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 4
	 Int(4) Arg: 0x8
	 Int(4) Arg: 0x7
	 Unsigned Arg: 32
	 Unsigned Arg: 0
	 Unsigned Arg: 4294967295
	 Unsigned Arg: 3
	 Unary(3) Arg: 0x0
	 Unary(3) Arg: 0x3
	 Unsigned Arg: 3
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x0 (false)
	 Unsigned Arg: 2
      */
      // minof, maxof related to lhs type ; sizeof always unsigned
      //constant fold minus
      return std::string("Exit status: 0\nUe_Fu { System s();  Int(32) test() {  s ( 4u )print . ; s ( -8 )print . s ( 7 )print . s ( 32u )print . s ( 0u )print . s ( 4294967295u )print . s ( 3u )print . s ( 0u )print . s ( 7u )print . s ( 3u )print . s ( false )print . s ( false )print . s ( 2u )print . 0 return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // similar to t3287 except using types instead of variables.
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt test(){\n s.print(Int(4).sizeof);\n;\n s.print(Int(4).minof);\n s.print(Int(4).maxof);\n s.print(Unsigned.sizeof);\n s.print(Unsigned.minof);\n s.print(Unsigned.maxof);\n s.print(Unary(3).sizeof);\n s.print(Unary(3).minof);\n s.print(Unary(3).maxof);\n s.print(Bool(3).sizeof);\n s.print(Bool(3).minof);\n s.print(Bool(3).maxof);\n s.print(Bits(2).sizeof);\n/* i = t.minof;\n */ return 0;\n}\n}\n");

      // uncovered problem with type of the values for maxof, minof, sizeof; not related to lhs type.
      //bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt test(){\ns.print(Bool(3).maxof);\n return 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3289_test_compiler_primitive_minmaxsizeof_bytype)

} //end MFM
