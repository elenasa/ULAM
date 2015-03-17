#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3287_test_compiler_primitive_minmaxsizeof)
  {
    std::string GetAnswerKey()
    {
      /* gen output:
	 Unsigned Arg: 4
	 Int Arg: -8
	 Int Arg: 7
	 Unsigned Arg: 4
	 Unsigned Arg: 0
	 Unsigned Arg: 15
	 Unsigned Arg: 4
	 Unsigned Arg: 0
	 Unsigned Arg: 4
	 Unsigned Arg: 3
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x7 (true)
	 Unsigned Arg: 2
      */

      //constant fold minus
      return std::string("Exit status: 0\nUe_Fu { System s();  Int(32) test() {  Unsigned(32) u;  Int(4) z;  u 4u = s ( u )print . Int(32) i;  i -8 = s ( i )print . s ( 7 )print . Unsigned(4) y;  u 4u = s ( u )print . s ( 0u )print . s ( 15u )print . Unary(4) x;  u 4u = s ( u )print . s ( 0u )print . s ( 4u )print . Bool(3) v;  u 3u = s ( u )print . s ( false cast )print . s ( true cast )print . Bits(2) t;  u 2u = s ( u )print . 0 return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt test(){\nUnsigned u;\nInt(4) z; u = z.sizeof;\n s.print(u);\nInt i = z.minof;\n s.print(i);\n s.print(z.maxof);\nUnsigned(4) y;\n u = y.sizeof;\n s.print(u);\n s.print(y.minof);\n s.print(y.maxof);\nUnary(4) x;\n u = x.sizeof;\n s.print(u);\n s.print(x.minof);\n s.print(x.maxof);\n Bool(3) v;\n u = v.sizeof;\n s.print(u);\n s.print(v.minof);\n s.print(v.maxof);\nBits(2) t;\n u = t.sizeof;\n s.print(u);\n/* i = t.minof;\n */ return 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3287_test_compiler_primitive_minmaxsizeof)

} //end MFM
