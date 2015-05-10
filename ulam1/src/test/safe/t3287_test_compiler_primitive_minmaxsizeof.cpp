#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3287_test_compiler_primitive_minmaxsizeof)
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

      //constant fold minus; constants of lhs type; return sizeof Void (== 0u)
      return std::string("Exit status: 0\nUe_Fu { System s();  Int(32) test() {  Int(4) z;  s ( 4u )print . s ( -8 )print . s ( 7 )print . Unsigned(32) y;  s ( 32u )print . s ( 0u )print . s ( 4294967295u )print . Unary(3) x;  s ( 3u )print . s ( 0u )print . s ( 7u )print . Bool(3) v;  s ( 3u )print . s ( 0u )print . s ( 7u )print . Bits(2) t;  s ( 2u )print . 0u cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt test(){\nInt(4) z; s.print(z.sizeof);\n s.print(z.minof);\n s.print(z.maxof);\nUnsigned y;\n s.print(y.sizeof);\n s.print(y.minof);\n s.print(y.maxof);\nUnary(3) x;\n s.print(x.sizeof);\n s.print(x.minof);\n s.print(x.maxof);\n Bool(3) v;\n s.print(v.sizeof);\n s.print(v.minof);\n s.print(v.maxof);\nBits(2) t;\n s.print(t.sizeof);\n /*i = t.minof;\n unsupported request minof Bits */  return Void.sizeof;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3287_test_compiler_primitive_minmaxsizeof)

} //end MFM
