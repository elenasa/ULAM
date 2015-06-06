#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3212_test_compiler_elementboolminusequal)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x0 (false)
	 Bool(3) Arg: 0x0 (false)
      */
      return std::string("Exit status: 0\nUe_Foo { Bool(3) b(true);  System s();  Unary(4) sp(0);  Bool(3) a(false);  Bool(3) c(false);  Int(32) test() {  a false cast = b true cast = c a cast b cast -b cast = s ( c )print . a b -= s ( a )print . a cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nUnary(4) sp;\nBool(3) a, b, c;\nInt test() {\na = false;\n b = true;\nc = a - b;\ns.print(c);\n a-=b;\ns.print(a);\n return a;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3212_test_compiler_elementboolminusequal)

} //end MFM
