#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3289_test_compiler_primitive_minmaxsizeof_bytype)
  {
    std::string GetAnswerKey()
    {
      /* gen output:
	 Unsigned Arg: 4
	 Int Arg: -8
	 Int(3) Arg: 0x3
	 Unsigned Arg: 4
	 Int Arg: 0
	 Unsigned Arg: 15
	 Unsigned Arg: 4
	 Int Arg: 0
	 Unsigned Arg: 4
	 Unsigned Arg: 3
	 Int Arg: 0
	 Bool(3) Arg: 0x7 (true)
	 Unsigned Arg: 2
      */

      //constant fold minus
      return std::string("Ue_Fu { System s();  Int(32) test() {  Unsigned(32) u;  Int(4) z;  u 4u cast = s ( u )print . Int(32) i;  i -8 cast = s ( i )print . s ( 7 cast )print . Unsigned(4) y;  u 4u cast = s ( u )print . i 0u cast = s ( i )print . s ( 15u cast )print . Unary(4) x;  u 4u cast = s ( u )print . i 0u cast = s ( i )print . s ( 4u cast )print . Bool(3) v;  u 3u cast = s ( u )print . i false cast = s ( i )print . s ( true cast )print . Bits(2) t;  u 2u cast = s ( u )print . 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt test(){\nUnsigned u;\nInt(4) z; u = Int(4).sizeof;\n s.print(u);\nInt i = Int(4).minof;\n s.print(i);\n s.print(Int(4).maxof);\nUnsigned(4) y;\n u = Unsigned(4).sizeof;\n s.print(u);\ni = Unsigned(4).minof;\n s.print(i);\n s.print(Unsigned(4).maxof);\nUnary(4) x;\n u = Unary(4).sizeof;\n s.print(u);\ni = Unary(4).minof;\n s.print(i);\n s.print(Unary(4).maxof);\n Bool(3) v;\n u = Bool(3).sizeof;\n s.print(u);\ni = Bool(3).minof;\n s.print(i);\n s.print(Bool(3).maxof);\nBits(2) t;\n u = Bits(2).sizeof;\n s.print(u);\n/* i = t.minof;\n */ return 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3289_test_compiler_primitive_minmaxsizeof_bytype)

} //end MFM


