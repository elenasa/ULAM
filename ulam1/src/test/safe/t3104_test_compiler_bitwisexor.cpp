#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x1
      */
      return std::string("Exit status: 1\nUe_A { Int(3) b(2);  System s();  Bool(1) d(true);  Int(3) a(1);  Int(32) test() {  a 3 = b 2 = a a cast b cast ^ cast = s ( a )print . d a 1 cast == = a cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","ulam 1; use System; element A {System s;\nBool d;\nInt(3) a, b;\nuse test;\na = 3;\nb = 2;\na = (Int(3)) ( a ^ b);\ns.print(a);\nd = (a == 1);\nreturn a;\n}\n}\n");
      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3104_test_compiler_bitwisexor)

} //end MFM
