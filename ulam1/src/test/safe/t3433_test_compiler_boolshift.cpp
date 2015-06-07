#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3433_test_compiler_boolshift)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x6 (true)
	 Bool(3) Arg: 0x3 (true)
	 Bool(3) Arg: 0x4 (false)
      */
      return std::string("Exit status: 0\nUe_A { System s();  Bool(3) u(true);  Bool(3) v(true);  Bool(3) z(false);  Int(32) test() {  v true cast = u v 1 cast << = v v 1 cast >> = z v 2 cast << = s ( u )print . s ( v )print . s ( z )print . 0 return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //conversion to Bits then shift then back to Bool. so, 7 << 1 drops to 6 (t); 7 >> 1 = 3 (t); 3 << 2 = 4 (f)
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(3) u, v, z;\n Int test() {\n v = true;\n u = v << 1;\n v = v >> 1;\n z = v << 2;\n s.print(u);\n s.print(v);\n s.print(z);\n return 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3433_test_compiler_boolshift)

} //end MFM
