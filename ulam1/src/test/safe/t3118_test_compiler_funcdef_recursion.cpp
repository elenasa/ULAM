#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 24
      */
      return std::string("Exit status: 24\nUe_A { System s();  Bool(7) d(false);  Int(32) a(24);  Int(32) test() {  a ( 4 )fact = s ( a )print . a return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nBool(7) d;\nInt test() {\na = fact(4);\ns.print(a);\nreturn a;\n}\nInt a;\nInt fact(Int n) {\nif(n>0)\nreturn (n * fact(n-1));\nelse\nreturn 1;\n}\n}\n");
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)

} //end MFM
