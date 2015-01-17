#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3299_test_compiler_funcdef_overload_diffrtn)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x3
	 Bool(3) Arg: 0x7 (true)
      */

      return std::string("Exit status: 0\nUe_A { System s();  Bool(1) sp(true);  Int(3) spi(3);  Int(32) test() {  Bool(1) mybool;  mybool true cast = spi ( mybool 7 cast )foo cast = s ( spi )print . sp ( mybool )foo = s ( sp cast )print . 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //different return types ok!
      bool rtn1 = fms->add("A.ulam","ulam 1;\nuse System;\nelement A {\nSystem s;\n Bool sp;\n Int(3) spi;\n Int foo(Bool b, Int i){\nreturn 3;\n}\n Bool foo(Bool b){\nreturn b;\n}\n Int foo(Int i){\nreturn i;\n}\n Int test() {\n Bool mybool = true;\nspi = foo(mybool, 7);\n s.print(spi);\n sp=foo(mybool);\n s.print((Bool(3)) sp);\n return 0;\n }\n}");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3299_test_compiler_funcdef_overload_diffrtn)

} //end MFM


