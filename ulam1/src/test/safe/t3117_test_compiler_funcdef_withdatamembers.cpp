#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  {
    std::string GetAnswerKey()
    {
      //Ue_A { Int(32) a(4);  Int(32) b(5);  Int(32) test() {  a 4 cast = b 5 cast = a b +b return } }\nExit status: 9
      return std::string("Ue_A { Int(4) b(5);  System s();  Int(4) a(4);  Bool(7) d(false);  Int(32) test() {  a 4 cast = b 5 cast = s ( a cast b cast +b )print . a cast b cast +b return } }\nExit status: 9");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A {  Int test(){ a=4; b=5; return (a + b); } Int a,b; }"); //allows data member var defined after use; return an expression
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {System s;\nInt test(){\na=4;\nb=5;\ns.print(a+b);\nreturn (a + b);\n}\nBool(7) d;\nInt(4) a,b;\n}\n"); //allows data member var defined after use; return an expression

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)

} //end MFM


