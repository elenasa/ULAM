#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(4) Arg: 0x7
      */
      // before new saturation answer:
      //Exit status: 9\nUe_A { Int(4) b(5);  System s();  Int(4) a(4);  Bool(7) d(false);  Int(32) test() {  a 4 cast = b 5 cast = s ( a cast b cast +b )print . a cast b cast +b return } }\nUq_System { <NOMAIN> }

      return std::string("Exit status: 7\nUe_A { Int(4) b(5);  System s();  Int(4) a(4);  Bool(7) d(false);  Int(32) test() {  a 4 cast = b 5 cast = s ( a b +b )print . a b +b cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //allows data member var defined after use; return an expression
      //answer is saturated
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nInt test(){\na=4;\nb=5;\ns.print(a+b);\nreturn (a + b);\n}\nBool(7) d;\nInt(4) a,b;\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)

} //end MFM
