#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3273_test_compiler_unsignedthreebit_plusequal_issue)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 1
	 Unsigned Arg: 1
	 Unsigned Arg: 1
	 Unsigned Arg: 2
	 Unsigned Arg: 2
	 Unsigned Arg: 2
	 Unsigned Arg: 3
	 Unsigned Arg: 3
	 Unsigned Arg: 3
	 Unsigned Arg: 4
	 Unsigned Arg: 4
	 Unsigned Arg: 4
	 Unsigned Arg: 5
	 Unsigned Arg: 5
	 Unsigned Arg: 5
	 Unsigned Arg: 6
	 Unsigned Arg: 6
	 Unsigned Arg: 6
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
	 Unsigned Arg: 7
      */

      return std::string("Exit status: 0\nUe_A { System s();  Int(32) test() {  Unsigned(3) x;  x 0 cast = Unsigned(3) y;  y 0 cast = Unsigned(3) z;  z 0 cast = { Int(32) i;  i 0 cast = i 12 cast < cond { x 1 cast += s ( x cast )print . y 1 cast += s ( y cast )print . z z cast cast 1 cast cast +b cast = s ( z cast )print . } _1: i 1 cast += while } 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // ++, += 1 and z = z + 1 all the same; saturates.
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nInt test(){ Unsigned(3) x = 0, y = 0, z = 0;\n for(Int i = 0; i < 12; ++i){\n x+=1;\n s.print((Unsigned) x);\n++y;\n s.print((Unsigned) y);\nz = (Unsigned(3)) (z + 1);\n s.print((Unsigned) z);\n }\n return 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3273_test_compiler_unsignedthreebit_plusequal_issue)

} //end MFM
